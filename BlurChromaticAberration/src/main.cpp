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
    PARAM_RADIUS_R,
    PARAM_RADIUS_G,
    PARAM_RADIUS_B,
    PARAM_MARGIN,
    PARAM_COUNT,
  };

  int param_count() const override { return PARAM_COUNT; }

  ParamPrototype const* param_prototype(int i) const override {
    static std::array<ParamPrototype, PARAM_COUNT> const params = {
        ParamPrototype{"gamma", PARAM_GROUP_DEFAULT, 2.2, 0.100, 5.0},
        ParamPrototype{"exposure", PARAM_GROUP_DEFAULT, 1.0, 0.125, 8.0},
        ParamPrototype{"radius_r", PARAM_GROUP_DEFAULT, 0.01, 0.000, 1.0},
        ParamPrototype{"radius_g", PARAM_GROUP_DEFAULT, 0.01, 0.000, 1.0},
        ParamPrototype{"radius_b", PARAM_GROUP_DEFAULT, 0.01, 0.000, 1.0},
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
    retrc.width += margin * 2 + 1;
    retrc.height += margin * 2 + 1;

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

  float const gamma = params.get<float>(PARAM_GAMMA);
  float const exposure = params.get<float>(PARAM_EXPOSURE);

  cv::Size const size = retimg.size();
  std::array<int, 3> const radius = {
      params.get<int>(PARAM_RADIUS_B, size.height * 0.1),
      params.get<int>(PARAM_RADIUS_G, size.height * 0.1),
      params.get<int>(PARAM_RADIUS_R, size.height * 0.1),
  };

  std::array<float, 3> const radius_sq = {
      tnzu::square(params.get<float>(PARAM_RADIUS_B, size.height * 0.1)),
      tnzu::square(params.get<float>(PARAM_RADIUS_G, size.height * 0.1)),
      tnzu::square(params.get<float>(PARAM_RADIUS_R, size.height * 0.1)),
  };

  DEBUG_PRINT("radius_r: " << radius[2]);
  DEBUG_PRINT("radius_g: " << radius[1]);
  DEBUG_PRINT("radius_b: " << radius[0]);
  DEBUG_PRINT("margin: " << params.get<int>(PARAM_MARGIN));

  // transform color space
  std::array<cv::Mat, 4> src = {
      cv::Mat(size, CV_32F), cv::Mat(size, CV_32F), cv::Mat(size, CV_32F),
      cv::Mat(size, CV_32F),
  };
  {
    tnzu::linear_color_space_converter<sizeof(value_type) * 8> converter(
        exposure, gamma);

    cv::Size const ssize = args.size(PORT_INPUT);
    std::array<cv::Mat, 4> local = {
        cv::Mat(src[0], cv::Rect(args.offset(PORT_INPUT), ssize)),
        cv::Mat(src[1], cv::Rect(args.offset(PORT_INPUT), ssize)),
        cv::Mat(src[2], cv::Rect(args.offset(PORT_INPUT), ssize)),
        cv::Mat(src[3], cv::Rect(args.offset(PORT_INPUT), ssize)),
    };

    float const alpha_scale = 1.0f / std::numeric_limits<value_type>::max();

#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (int y = 0; y < ssize.height; y++) {
      Vec4T const* s = args.get(PORT_INPUT).ptr<Vec4T const>(y);
      std::array<float*, 4> d = {
          local[0].ptr<float>(y), local[1].ptr<float>(y),
          local[2].ptr<float>(y), local[3].ptr<float>(y),
      };
      for (int x = 0; x < ssize.width; x++) {
        // color2power
        d[0][x] = converter[s[x][0]];
        d[1][x] = converter[s[x][1]];
        d[2][x] = converter[s[x][2]];
        d[3][x] = s[x][3] * alpha_scale;
      }
    }
  }

  DEBUG_PRINT(__LINE__);

  std::array<cv::Mat, 6> dst;
#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int c = 0; c < 3; ++c) {
    // make filter kernel
    int const r = radius[c];
    int const fsize = r * 2 + 1;

    cv::Mat kernel = cv::Mat::zeros(cv::Size(fsize, fsize), CV_32F);
    if (r > 0) {
      cv::circle(kernel, cv::Point(r, r), r, cv::Scalar(1), -1);
      kernel /= (radius_sq[c] * M_PI);
    } else {
      kernel = cv::Scalar(1);
    }

    // apply filter
    cv::filter2D(src[c], dst[c + 0], -1, kernel);
    cv::filter2D(src[3], dst[c + 3], -1, kernel);
  }

  DEBUG_PRINT(__LINE__);
// transform color space
#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int y = 0; y < size.height; ++y) {
    std::array<float const*, 6> s = {
        dst[0].ptr<float const>(y), dst[1].ptr<float const>(y),
        dst[2].ptr<float const>(y), dst[3].ptr<float const>(y),
        dst[4].ptr<float const>(y), dst[5].ptr<float const>(y),
    };
    Vec4T* d = retimg.ptr<Vec4T>(y);
    for (int x = 0; x < size.width; ++x) {
      // sRGB (straight alpha)
      cv::Vec4f const sbgra(
          tnzu::to_nonlinear_color_space(s[0][x], exposure, gamma),
          tnzu::to_nonlinear_color_space(s[1][x], exposure, gamma),
          tnzu::to_nonlinear_color_space(s[2][x], exposure, gamma),
          (s[3][x] * (1.0f - s[4][x]) + s[4][x]) * (1.0f - s[5][x]) + s[5][x]);

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
