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
    PORT_MASK,
    PORT_COUNT,
  };

  int port_count() const override { return PORT_COUNT; }

  char const* port_name(int i) const override {
    static std::array<char const*, PORT_COUNT> names = {
        "Input", "Mask",
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
    PARAM_RADIUS_R,
    PARAM_RADIUS_G,
    PARAM_RADIUS_B,
    PARAM_RADIUS_A,
    PARAM_COUNT,
  };

  int param_count() const override { return PARAM_COUNT; }

  ParamPrototype const* param_prototype(int i) const override {
    static std::array<ParamPrototype const, PARAM_COUNT> params = {
        ParamPrototype{"radius_r", PARAM_GROUP_DEFAULT, 1, 0, 16},
        ParamPrototype{"radius_g", PARAM_GROUP_DEFAULT, 1, 0, 16},
        ParamPrototype{"radius_b", PARAM_GROUP_DEFAULT, 1, 0, 16},
        ParamPrototype{"radius_a", PARAM_GROUP_DEFAULT, 1, 0, 16},
    };
    return &params[i];
  }

 public:
  template <typename Vec4T>
  int kernel(Params const& params, Args const& args, cv::Mat& retimg);

  int compute(Config const& config, Params const& params, Args const& args,
              cv::Mat& retimg) override try {
    DEBUG_PRINT(__FUNCTION__);
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

  float const gamma = 2.2f;

  tnzu::linear_color_space_converter<sizeof(value_type) * 8> converter(1.0f,
                                                                       gamma);
  float const alpha_scale = 1.0f / std::numeric_limits<value_type>::max();

  cv::Size const size = retimg.size();

  std::array<float, 4> const radius = {
      params.get<float>(PARAM_RADIUS_B) * alpha_scale,
      params.get<float>(PARAM_RADIUS_G) * alpha_scale,
      params.get<float>(PARAM_RADIUS_R) * alpha_scale,
      params.get<float>(PARAM_RADIUS_A) * alpha_scale,
  };

  cv::Mat input(retimg.size(), retimg.type(), cv::Scalar(0, 0, 0, 0));
  if (args.valid(PORT_INPUT)) {
    args.get(PORT_INPUT).copyTo(input(args.rect(PORT_INPUT)));
  }

  cv::Mat mask(retimg.size(), retimg.type(), cv::Scalar(0, 0, 0, 0));
  if (args.valid(PORT_MASK)) {
    args.get(PORT_MASK).copyTo(mask(args.rect(PORT_MASK)));
  }

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int y = 0; y < size.height; ++y) {
    Vec4T const* m = mask.ptr<Vec4T const>(y);
    Vec4T* d = retimg.ptr<Vec4T>(y);

    for (int x = 0; x < size.width; ++x) {
      std::array<float, 4> const r = {
          m[x][0] * radius[0], m[x][1] * radius[1], m[x][2] * radius[2],
          m[x][3] * radius[3],
      };

      std::array<float, 4> const r2 = {
          tnzu::square(r[0]), tnzu::square(r[1]), tnzu::square(r[2]),
          tnzu::square(r[3]),
      };

      int const dr = static_cast<int>(
          std::ceil(std::max(std::max(r[0], r[1]), std::max(r[2], r[3]))));

      cv::Vec4f color(0, 0, 0, 0);
      cv::Vec4i accum(0, 0, 0, 0);

      for (int dy = -dr; dy <= dr; ++dy) {
        int const yy =
            cv::borderInterpolate(y + dy, size.height, cv::BORDER_WRAP);
        for (int dx = -dr; dx <= dr; ++dx) {
          int const xx =
              cv::borderInterpolate(x + dx, size.width, cv::BORDER_WRAP);

          Vec4T const texel = input.at<Vec4T>(yy, xx);

          int const d2 = dy * dy + dx * dx;
          for (int c = 0; c < 3; ++c) {
            if (d2 <= r2[c]) {
              color[c] += converter[texel[c]];
              accum[c]++;
            }
          }
          {
            int const c = 3;
            if (d2 <= r2[c]) {
              color[c] += texel[c];
              accum[c]++;
            }
          }
        }
      }

      for (int c = 0; c < 3; ++c) {
        if (accum[c] > 0) {
          d[x][c] = tnzu::normalize_cast<value_type>(
              tnzu::to_nonlinear_color_space(color[c] / accum[c], 1.0f, gamma));
        }
      }
      {
        int const c = 3;
        if (accum[c] > 0) {
          d[x][c] = cv::saturate_cast<value_type>(color[c] / accum[c]);
        }
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
