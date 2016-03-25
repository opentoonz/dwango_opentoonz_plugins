#define TNZU_DEFINE_INTERFACE
#include <toonz_utility.hpp>
#include <opencv2/imgproc/imgproc.hpp>

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
    PARAM_GROUP_GEOMETRY,
    PARAM_GROUP_COLOR,
    PARAM_GROUP_COUNT,
  };

  int param_group_count() const override { return PARAM_GROUP_COUNT; }

  char const* param_group_name(int i) const override {
    static std::array<char const*, PARAM_GROUP_COUNT> names = {
        "Geometry", "Color",
    };
    return names[i];
  }

  //
  // PARAM
  //
  enum {
    PARAM_GEOMETRY_DISTANCE,
    PARAM_GEOMETRY_THETA,
    PARAM_GEOMETRY_RADIUS,
    PARAM_GEOMETRY_RED,
    PARAM_GEOMETRY_GREEN,
    PARAM_GEOMETRY_BLUE,
    PARAM_COUNT,
  };

  int param_count() const override { return PARAM_COUNT; }

  ParamPrototype const* param_prototype(int i) const override {
    static std::array<ParamPrototype, PARAM_COUNT> const params = {
        ParamPrototype{"distance", PARAM_GROUP_GEOMETRY, -1, -1.5, 1.5},
        ParamPrototype{"theta", PARAM_GROUP_GEOMETRY, 40, -180.0, 180.0},
        ParamPrototype{"radius", PARAM_GROUP_GEOMETRY, 0.1, 0.0, 1},
        ParamPrototype{"red", PARAM_GROUP_COLOR, 0, 0.0, 1.0},
        ParamPrototype{"green", PARAM_GROUP_COLOR, 0, 0.0, 1.0},
        ParamPrototype{"blue", PARAM_GROUP_COLOR, 0, 0.0, 1.0},
    };
    return &params[i];
  }

 public:
  int enlarge(Config const& config, Params const& params,
              cv::Rect2d& retrc) override {
    DEBUG_PRINT(__FUNCTION__);
    // this is a fllscreen effect
    retrc = tnzu::make_infinite_rect<double>();
    return 0;
  }

  template <typename Vec4T>
  int kernel(cv::Size size, cv::Mat const& shadow, cv::Mat& retimg);

  int compute(Config const& config, Params const& params, Args const& args,
              cv::Mat& retimg) override try {
    DEBUG_PRINT(__FUNCTION__);

    if (args.invalid(PORT_INPUT)) {
      return 0;
    }

    cv::Size const size = retimg.size();

    //
    // Params
    //
    // geometry
    float const d = params.get<float>(PARAM_GEOMETRY_DISTANCE, size.height);
    float const a = params.radian<float>(PARAM_GEOMETRY_THETA);
    int const s =
        params.get<int>(PARAM_GEOMETRY_RADIUS, size.height * 0.5) * 2 + 1;
    float const r = params.get<float>(PARAM_GEOMETRY_RED);
    float const g = params.get<float>(PARAM_GEOMETRY_GREEN);
    float const b = params.get<float>(PARAM_GEOMETRY_BLUE);

    // draw a background
    tnzu::draw_image(retimg, args.get(PORT_INPUT), args.offset(PORT_INPUT));

    // define paraffin shadow
    cv::Mat shadow(size, CV_32FC3, cv::Scalar(1, 1, 1, 1));

    // draw parafffin
    {
      std::array<cv::Point, 4> pts;

      cv::Point2f const o(size.width * 0.5f, size.height * 0.5f);
      float const l = std::sqrt(o.x * o.x + o.y * o.y) + 1;
      float const s = std::sin(a);
      float const c = std::cos(a);

      pts[0] = cv::Point(static_cast<int>(o.x + l * c + (d + l) * s),
                         static_cast<int>(o.y - l * s + (d + l) * c));
      pts[1] = cv::Point(static_cast<int>(o.x + l * c + (d - l) * s),
                         static_cast<int>(o.y - l * s + (d - l) * c));
      pts[2] = cv::Point(static_cast<int>(o.x - l * c + (d - l) * s),
                         static_cast<int>(o.y + l * s + (d - l) * c));
      pts[3] = cv::Point(static_cast<int>(o.x - l * c + (d + l) * s),
                         static_cast<int>(o.y + l * s + (d + l) * c));

      cv::fillConvexPoly(shadow, pts.data(), static_cast<int>(pts.size()),
                         cv::Scalar(b, g, r));
    }

    // blur bar
    cv::GaussianBlur(shadow, shadow, cv::Size(s, s), 0.0);

    if (retimg.type() == CV_8UC4) {
      return kernel<cv::Vec4b>(size, shadow, retimg);
    } else {
      return kernel<cv::Vec4w>(size, shadow, retimg);
    }
  } catch (cv::Exception const& e) {
    DEBUG_PRINT(e.what());
  }
};

template <typename Vec4T>
int MyFx::kernel(cv::Size size, cv::Mat const& shadow, cv::Mat& retimg) {
  using value_type = typename Vec4T::value_type;

  // init color table
  tnzu::linear_color_space_converter<sizeof(value_type) * 8> converter(1.0f,
                                                                       2.2f);

// add incident light on linear color space
#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int y = 0; y < size.height; y++) {
    cv::Vec3f const* s = shadow.ptr<cv::Vec3f>(y);
    Vec4T* d = retimg.ptr<Vec4T>(y);
    for (int x = 0; x < size.width; x++) {
      cv::Vec4f const sbgra(tnzu::to_nonlinear_color_space(
                                converter[d[x][0]] * s[x][0], 1.0f, 2.2f),
                            tnzu::to_nonlinear_color_space(
                                converter[d[x][1]] * s[x][1], 1.0f, 2.2f),
                            tnzu::to_nonlinear_color_space(
                                converter[d[x][2]] * s[x][2], 1.0f, 2.2f),
                            1.0f);
      for (int c = 0; c < 4; ++c) {
        d[x][c] = tnzu::normalize_cast<value_type>(sbgra[c]);
      }
    }
  }
}

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
