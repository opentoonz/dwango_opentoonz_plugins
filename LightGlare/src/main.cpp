#define TNZU_DEFINE_INTERFACE
#include <toonz_utility.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

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
    PARAM_GAMMA,
    PARAM_EXPOSURE,
    PARAM_GAIN,
    PARAM_RADIUS,
    PARAM_ATTENUATION,
    PARAM_NUMBER,
    PARAM_ANGLE,
    PARAM_MARGIN,
    PARAM_COUNT,
  };

  int param_count() const override { return PARAM_COUNT; }

  ParamPrototype const* param_prototype(int i) const override {
    static std::array<ParamPrototype, PARAM_COUNT> const params = {
        ParamPrototype{"gamma", PARAM_GROUP_DEFAULT, 2.2, 0.100, 5},
        ParamPrototype{"exposure", PARAM_GROUP_DEFAULT, 1.0, 0.125, 8},
        ParamPrototype{"gain", PARAM_GROUP_DEFAULT, 1.0, 0.100, 10},
        ParamPrototype{"radius", PARAM_GROUP_DEFAULT, 0.1, 0.01, 1},
        ParamPrototype{"attenuation", PARAM_GROUP_DEFAULT, 0.9, 0.001, 0.999},
        ParamPrototype{"number", PARAM_GROUP_DEFAULT, 6.0, 2.000, 10},
        ParamPrototype{"angle", PARAM_GROUP_DEFAULT, 15.0, 0.000, 180},
        ParamPrototype{"margin", PARAM_GROUP_DEFAULT, 100, 0, 1024},
    };
    return &params[i];
  }

 public:
  int enlarge(Config const& config, Params const& params,
              cv::Rect2d& retrc) override {
    DEBUG_PRINT(__FUNCTION__);

    int const margin = params.get<int>(PARAM_MARGIN);
    retrc.x -= margin;
    retrc.y -= margin;
    retrc.width += margin * 2;
    retrc.height += margin * 2;

    return 0;
  }

  template <typename Vec4T>
  int kernel(Params const& params, Args const& args, cv::Mat& retimg);

  int compute(Config const& config, Params const& params, Args const& args,
              cv::Mat& retimg) override try {
    DEBUG_PRINT(__FUNCTION__);
    if (args.invalid(PORT_INPUT)) {
      return 0;
    }

    if (retimg.type() == CV_8UC4) {
      return kernel<cv::Vec4b>(params, args, retimg);
    } else {
      return kernel<cv::Vec4w>(params, args, retimg);
    }
  } catch (cv::Exception const& e) {
    DEBUG_PRINT(e.what());
  }
};

template <typename Vec4T>
int MyFx::kernel(Params const& params, Args const& args, cv::Mat& retimg) {
  using value_type = typename Vec4T::value_type;

  cv::Size const size = retimg.size();

  // init parameters
  float const gamma = params.get<float>(PARAM_GAMMA);
  float const exposure = params.get<float>(PARAM_EXPOSURE);
  float const gain = params.get<float>(PARAM_GAIN);

  int const radius = params.get<int>(PARAM_RADIUS, size.height);
  float const attenuation = params.get<float>(PARAM_ATTENUATION);
  int const number = params.get<int>(PARAM_NUMBER);
  float const angle = params.radian<float>(PARAM_ANGLE);

  cv::Mat src(size, CV_32FC3);

  // transform color space
  DEBUG_PRINT("transform color space");
  {
    tnzu::linear_color_space_converter<sizeof(value_type) * 8> converter(
        exposure, gamma);

    cv::Size const local_size = args.size(PORT_INPUT);
    cv::Mat local(src, args.rect(PORT_INPUT));

#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (int y = 0; y < local_size.height; ++y) {
      Vec4T const* s = args.get(PORT_INPUT).ptr<Vec4T const>(y);
      cv::Vec3f* d = local.ptr<cv::Vec3f>(y);
      for (int x = 0; x < local_size.width; ++x) {
        d[x] = cv::Vec3f(converter[s[x][0]], converter[s[x][1]],
                         converter[s[x][2]]);
      }
    }
  }

  // generate glare kernel
  DEBUG_PRINT("generate glare kernel: " << radius);
  int const fsize = radius * 2 + 1;
  cv::Mat kernel = cv::Mat::zeros(cv::Size(fsize, fsize), CV_32F);
  if (radius > 0) {
    float energy = 0.0f;
    cv::Point2f const center(fsize * 0.5f, fsize * 0.5f);
    for (int i = 0; i < number; i++) {
      float const theta = angle + i * float(2 * M_PI) / number;
      float const dx = radius * std::cos(theta);
      float const dy = radius * std::sin(theta);

      cv::LineIterator it(kernel, center, center + cv::Point2f(dx, dy));
      float a = 1.0f;
      for (int i = 0; i < it.count; ++i, ++it, a *= attenuation) {
        *reinterpret_cast<float*>(*it) = a;
        energy += a;
      }
    }
    if (energy > 0.0f) {
      kernel *= gain / energy;
    }
  } else {
    kernel = cv::Scalar(1);
  }

  // generate glare
  DEBUG_PRINT("generate glare");
  cv::filter2D(src, src, -1, kernel);

  // transform color space
  DEBUG_PRINT("transform color space");
#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int y = 0; y < size.height; ++y) {
    cv::Vec3f const* s = src.ptr<cv::Vec3f>(y);
    Vec4T* d = retimg.ptr<Vec4T>(y);
    for (int x = 0; x < size.width; ++x) {
      // sRGB (straight alpha)
      cv::Vec4f const sbgra(
          tnzu::to_nonlinear_color_space(s[x][0], exposure, gamma),
          tnzu::to_nonlinear_color_space(s[x][1], exposure, gamma),
          tnzu::to_nonlinear_color_space(s[x][2], exposure, gamma), 1.0f);

      for (int c = 0; c < 4; ++c) {
        d[x][c] = tnzu::normalize_cast<value_type>(sbgra[c]);
      }
    }
  }

  return 0;
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
