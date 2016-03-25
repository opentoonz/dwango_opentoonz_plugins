#define TNZU_DEFINE_INTERFACE
#include <toonz_utility.hpp>
#include <opencv2/imgproc/imgproc.hpp>

struct plane_t {
  cv::Point3d n;
  double d;

  inline double find_intersection(cv::Point3d const& origin,
                                  cv::Point3d const& direction) {
    double const dot = -n.dot(direction);
    if (dot < std::numeric_limits<double>::epsilon()) {
      return std::numeric_limits<double>::infinity();
    } else {
      return (n.dot(origin) + d) / dot;
    }
  }
};

class MyFx : public tnzu::Fx {
 public:
  //
  // PORT
  //
  enum {
    PORT_INPUT,
    PORT_COUNT,
  };

  int port_count() const override { return PORT_COUNT; }

  char const* port_name(int i) const override {
    static std::array<char const*, PORT_COUNT> names = {
        "Input",
    };
    return names[i];
  }

  //
  // PARAM GROUP
  //
  enum {
    PARAM_GROUP_DEFAULT,
    PARAM_GROUP_COUNT,
  };

  int param_group_count() const override { return PARAM_GROUP_COUNT; }

  char const* param_group_name(int i) const override {
    static std::array<char const*, PARAM_GROUP_COUNT> names = {
        "Default",
    };
    return names[i];
  }

  //
  // PARAM
  //
  enum {
    PARAM_NUMBER,
    PARAM_ANGLE,
    PARAM_X,
    PARAM_Y,
    PARAM_RADIUS,
    PARAM_ALBEDO,
    PARAM_DEPTH,
    PARAM_COUNT
  };

  int param_count() const override { return PARAM_COUNT; }

  ParamPrototype const* param_prototype(int i) const override {
    static std::array<ParamPrototype, PARAM_COUNT> const params = {
        ParamPrototype{"number", PARAM_GROUP_DEFAULT, 3.0, 3.00, 10.00},
        ParamPrototype{"angle", PARAM_GROUP_DEFAULT, 0.0, 0.00, 360.00},
        ParamPrototype{"x", PARAM_GROUP_DEFAULT, 0.5, 0.00, 1.00},
        ParamPrototype{"y", PARAM_GROUP_DEFAULT, 0.5, 0.00, 1.00},
        ParamPrototype{"radius", PARAM_GROUP_DEFAULT, 0.5, 0.00, 1.00},
        ParamPrototype{"albedo", PARAM_GROUP_DEFAULT, 0.7, 0.01, 0.99},
        ParamPrototype{"depth", PARAM_GROUP_DEFAULT, 10.0, 0.00, 100.00},
    };
    return &params[i];
  }

 public:
  int enlarge(Config const& config, Params const& params,
              cv::Rect2d& retrc) override {
    DEBUG_PRINT(__FUNCTION__);
    // this is a fullscreen effect
    retrc = tnzu::make_infinite_rect<double>();
    return 0;
  }

  int compute(Config const& config, Params const& params, Args const& args,
              cv::Mat& retimg) override try {
    DEBUG_PRINT(__FUNCTION__);

    if (args.invalid(PORT_INPUT)) {
      return 0;
    }

    cv::Size const size = retimg.size();
    int const type = retimg.type();

    // cliping
    cv::Mat src(size, type);
    tnzu::draw_image(src, args.get(PORT_INPUT), args.offset(PORT_INPUT));

    // init parameters
    DEBUG_PRINT("init parameters");
    int const number = params.get<int>(PARAM_NUMBER);
    double const angle = params.radian<float>(PARAM_ANGLE);
    double const cx = params.get<double>(PARAM_X, size.width);  // center (x, y)
    double const cy = params.get<double>(PARAM_Y, size.height);
    double const radius = params.get<double>(PARAM_RADIUS, size.height);
    double const albedo = params.get<double>(PARAM_ALBEDO);
    int const depth = params.get<int>(PARAM_DEPTH);

    // build kaleidoscope mirrors
    DEBUG_PRINT("build kaleidoscope mirrors");
    std::vector<plane_t> planes(number + 1);
    planes[0].n = cv::Point3d(0, 0, 1);
    planes[0].d = 0;
    for (int i = 1; i <= number; ++i) {
      double const theta = angle + (i - 1) * (2 * M_PI) / number;
      double const nx = std::cos(theta);
      double const ny = std::sin(theta);
      planes[i].n = cv::Point3d(nx, ny, 0);

      // x is a point on the plane
      cv::Point3d const x(cx - radius * nx, cy - radius * ny, 0);
      planes[i].d = -planes[i].n.dot(x);
    }

    // generate kaleidoscope view
    DEBUG_PRINT("generate kaleidoscope view");
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (int y = 0; y < size.height; ++y) {
      for (int x = 0; x < size.width; ++x) {
        // init view ray
        double rho = 1;
        cv::Point3d origin(cx, cy, 1);
        cv::Point3d const target(x, y, 0);
        cv::Point3d direction = target - origin;
        direction /= cv::norm(direction);

        cv::Point2d tap_pos(std::numeric_limits<float>::infinity(),
                            std::numeric_limits<float>::infinity());

        // trace ray (max iterate: depth)
        for (int i = 0; i < depth; ++i) {
          // find intersection
          double min_distance = planes[0].find_intersection(origin, direction);
          int near_plane_i = 0;
          for (int i = 1; i <= number; ++i) {
            double const distance =
                planes[i].find_intersection(origin, direction);
            if ((distance > 1e-8) && (min_distance > distance)) {
              min_distance = distance;
              near_plane_i = i;
            }
          }
          if (near_plane_i < 0) {
            break;  // fatal
          }
          // generate a reflect ray
          origin = tnzu::meet(origin, direction, min_distance);
          direction = tnzu::reflect(direction, planes[near_plane_i].n);

          if (near_plane_i == 0) {
            tap_pos.x = origin.x;
            tap_pos.y = origin.y;
            break;  // success
          }

          rho *= albedo;
        }

        // skip this pixel if it failed to trace a ray
        if (!std::isfinite(tap_pos.x) || !std::isfinite(tap_pos.y)) {
          continue;
        }

        if ((tap_pos.x < 0) || (tap_pos.x >= src.cols) || (tap_pos.y < 0) ||
            (tap_pos.y >= src.rows)) {
          continue;  // out of screen
        }

        if (type == CV_8UC4) {
          // tap src
          cv::Vec4b data = tnzu::tap_texel<cv::Vec4b>(src, tap_pos);
          for (int c = 0; c < 3; ++c) {
            data[c] = cv::saturate_cast<uchar>(data[c] * rho);
          }

          // record gbra
          retimg.at<cv::Vec4b>(y, x) = data;
        } else {
          // tap src
          cv::Vec4w data = tnzu::tap_texel<cv::Vec4w>(src, tap_pos);
          for (int c = 0; c < 3; ++c) {
            data[c] = cv::saturate_cast<ushort>(data[c] * rho);
          }

          // record gbra
          retimg.at<cv::Vec4w>(y, x) = data;
        }
      }
    }

    return 0;
  } catch (cv::Exception const& e) {
    DEBUG_PRINT(e.what());
  }
};

namespace tnzu {
PluginInfo const* plugin_info() {
  static PluginInfo const info(TNZU_PP_STR(PLUGIN_NAME),    // name
                               TNZU_PP_STR(PLUGIN_VENDOR),  // vendor
                               "",                          // note
                               "http://dwango.co.jp/");     // helpurl
  return &info;
}

Fx* make_fx() { return new MyFx(); }
}
