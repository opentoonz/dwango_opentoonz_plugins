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
    PORT_NOISE,
    PORT_MASK,
    PORT_COUNT,
  };

  int port_count() const override { return PORT_COUNT; }

  char const* port_name(int i) const override {
    static std::array<char const*, PORT_COUNT> names = {
        "Input", "Noise", "Mask",
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
    PARAM_GAIN,
    PARAM_ATTENUATION,
    PARAM_DEBUG,
    PARAM_COUNT,
  };

  int param_count() const override { return PARAM_COUNT; }

  ParamPrototype const* param_prototype(int i) const override {
    static std::array<ParamPrototype, PARAM_COUNT> const params = {
        ParamPrototype{"gain", PARAM_GROUP_DEFAULT, 16, 0, 2},
        ParamPrototype{"attenuation", PARAM_GROUP_DEFAULT, 0.9, 0, 1},
        ParamPrototype{"debug", PARAM_GROUP_DEFAULT, 0, 0, 1},
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

  template <typename Vec4T>
  int kernel(Params const& params, Args const& args, cv::Mat& retimg);

  int compute(Config const& config, Params const& params, Args const& args,
              cv::Mat& retimg) override try {
    DEBUG_PRINT(__FUNCTION__);
    if (args.invalid(PORT_INPUT) || args.invalid(PORT_NOISE)) {
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
  cv::Size const size = retimg.size();
  int const type = retimg.type();

  using value_type = typename Vec4T::value_type;

  float const gain = params.get<float>(PARAM_GAIN);
  float const attenuation = params.get<float>(PARAM_ATTENUATION);
  bool const debug = params.get<bool>(PARAM_DEBUG);

  // generate a potencial field by perlin noise
  cv::Mat field = args.get(PORT_NOISE);
  if (debug) {
    for (int y = 0; y < size.height; ++y) {
      float const* src = field.ptr<float>(y);
      Vec4T* dst = retimg.ptr<Vec4T>(y);
      for (int x = 0; x < size.width; ++x) {
        int const g = tnzu::normalize_cast<value_type>(src[x] * gain);
        dst[x] = Vec4T(g, g, g, std::numeric_limits<value_type>::max());
      }
    }
    return 0;
  }

  // mask
  if (args.valid(PORT_MASK)) {
    cv::Mat mask = cv::Mat::zeros(field.size(), type);
    tnzu::draw_image(mask, args.get(PORT_MASK), args.offset(PORT_MASK));

    for (int y = 0; y < mask.rows; ++y) {
      Vec4T const* mv = mask.ptr<Vec4T>(y);
      float* fv = field.ptr<float>(y);
      for (int x = 0; x < mask.cols; ++x) {
        if (mv[x][3]) {
          fv[x] = 0.0f;
        }
      }
    }
  }

  // src
  cv::Mat src = cv::Mat::zeros(size, type);
  tnzu::draw_image(src, args.get(PORT_INPUT), args.offset(PORT_INPUT));

  float const length = gain * size.height * 0.5f;
  DEBUG_PRINT("length = " << length);

// generate curl noise
#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int y = 0; y < size.height; ++y) {
    Vec4T* dst = retimg.ptr<Vec4T>(y);

    int const y0 = cv::borderInterpolate(y - 1, field.rows, cv::BORDER_WRAP);
    int const y1 = cv::borderInterpolate(y + 1, field.rows, cv::BORDER_WRAP);

    for (int x = 0; x < size.width; ++x) {
      int const x0 = cv::borderInterpolate(x - 1, field.cols, cv::BORDER_WRAP);
      int const x1 = cv::borderInterpolate(x + 1, field.cols, cv::BORDER_WRAP);

      // calculate gradient by central difference
      float const dx = field.at<float>(y, x1) - field.at<float>(y, x0);
      float const dy = field.at<float>(y1, x) - field.at<float>(y0, x);

      // calculate a velocity of flow at (x,y) by curl operation
      float const vx = dy * length;
      float const vy = -dx * length;

      cv::Vec4f color(0, 0, 0, 0);

      // line integral convolution
      cv::Point const origin(x, y);
      float sum = 0.0f;
      {
        // minus
        cv::Point const pt(static_cast<int>(x - vx), static_cast<int>(y - vy));
        cv::LineIterator it(src, origin, pt, 4);
        if (it.count > 0) {
          float rho = 1.0f;
          for (int i = 0; i < it.count; ++i, ++it, rho *= attenuation) {
            Vec4T const sample = *reinterpret_cast<Vec4T const*>(*it);
            for (int c = 0; c < 4; ++c) {
              color[c] += sample[c] * rho;
            }
            sum += rho;
          }
        }
      }
      {
        // plus
        cv::Point const pt(static_cast<int>(x + vx), static_cast<int>(y + vy));
        cv::LineIterator it(src, origin, pt, 4);
        if (it.count > 0) {
          float rho = 1.0f;
          for (int i = 0; i < it.count; ++i, ++it, rho *= attenuation) {
            Vec4T const sample = *reinterpret_cast<Vec4T const*>(*it);
            for (int c = 0; c < 4; ++c) {
              color[c] += sample[c] * rho;
            }
            sum += rho;
          }
        }
      }
      if (sum > 0) {
        color /= sum;
      }
      dst[x] = color;
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
