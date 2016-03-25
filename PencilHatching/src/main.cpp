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
    PARAM_GROUP_SYSTEM,
    PARAM_GROUP_COUNT,
  };

  int param_group_count() const override { return PARAM_GROUP_COUNT; }

  char const* param_group_name(int i) const override {
    static std::array<char const*, PARAM_GROUP_COUNT> names = {
        "Default", "System",
    };
    return names[i];
  }

  //
  // PARAM
  //
  enum {
    PARAM_ANGLE,
    PARAM_LENGTH,
    PARAM_ATTENUATION,
    PARAM_SEED,
    PARAM_COUNT,
  };

  int param_count() const override { return PARAM_COUNT; }

  ParamPrototype const* param_prototype(int i) const override {
    static std::array<ParamPrototype, PARAM_COUNT> const params = {
        ParamPrototype{"angle", PARAM_GROUP_DEFAULT, 0.00, 0, 360},
        ParamPrototype{"length", PARAM_GROUP_DEFAULT, 0.01, 0, 1},
        ParamPrototype{"attenuation", PARAM_GROUP_DEFAULT, 0.90, 0, 1},
        ParamPrototype{"seed", PARAM_GROUP_SYSTEM, 0.25, 0, 1},
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
  int const type = retimg.type();

  cv::Size const size = retimg.size();

  float const angle = params.radian<float>(PARAM_ANGLE);
  float const length = params.get<float>(PARAM_LENGTH, size.height);
  float const attenuation = params.get<float>(PARAM_ATTENUATION);

  // snp noise based on grayscale
  cv::Mat noise(size, CV_MAKETYPE(CV_MAT_DEPTH(type), 1));
  cv::Mat color = cv::Mat::zeros(size, type);
  {
    // copy
    tnzu::draw_image(color, args.get(PORT_INPUT), args.offset(PORT_INPUT));

    // grascaling
    cv::Mat grayscale;
    cv::cvtColor(color, grayscale, cv::COLOR_BGRA2GRAY);

    float const norm_const = 1.0f / std::numeric_limits<value_type>::max();

    // generate snp noise
    std::mt19937_64 engine = params.rng<std::uint64_t>(PARAM_SEED);
    for (int y = 0; y < size.height; ++y) {
      value_type const* g = grayscale.ptr<value_type const>(y);
      value_type* n = noise.ptr<value_type>(y);
      for (int x = 0; x < size.width; ++x) {
        std::bernoulli_distribution rbern(g[x] * norm_const);
        n[x] = rbern(engine) ? std::numeric_limits<value_type>::max() : 0;
      }
    }
  }

  // generate pencil drawings
  cv::Point2f const dir(std::cos(angle), std::sin(angle));
#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int y = 0; y < size.height; ++y) {
    Vec4T const* src = color.ptr<Vec4T>(y);
    Vec4T* dst = retimg.ptr<Vec4T>(y);

    for (int x = 0; x < size.width; ++x) {
      // line integral convolution
      cv::Point const org(x, y);  // origin
      float gray = 0.0f;
      float sum = 0.0f;
      {
        // minus
        cv::Point const pt(static_cast<int>(x - length * dir.x),
                           static_cast<int>(y - length * dir.y));
        cv::LineIterator it(noise, org, pt, 4);
        if (it.count > 0) {
          float rho = 1.0f;
          for (int i = 0; i < it.count; ++i, ++it, rho *= attenuation) {
            value_type const sample = *reinterpret_cast<value_type const*>(*it);
            gray += rho * sample;
            sum += rho;
          }
        }
      }
      {
        // plus
        cv::Point const pt(static_cast<int>(x + length * dir.x),
                           static_cast<int>(y + length * dir.y));
        cv::LineIterator it(noise, org, pt, 4);
        if (it.count > 0) {
          float rho = 1.0f;
          for (int i = 0; i < it.count; ++i, ++it, rho *= attenuation) {
            value_type const sample = *reinterpret_cast<value_type const*>(*it);
            gray += rho * sample;
            sum += rho;
          }
        }
      }
      if (sum > 0) {
        gray /= sum;
      }
      value_type const g = cv::saturate_cast<value_type>(gray);
      dst[x] = Vec4T(g, g, g, src[x][3]);
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
