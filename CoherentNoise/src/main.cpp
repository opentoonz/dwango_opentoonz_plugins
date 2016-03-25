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
    PARAM_GROUP_GLOBAL,
    PARAM_GROUP_FIELD,
    PARAM_GROUP_SYSTEM,
    PARAM_GROUP_COUNT,
  };

  int param_group_count() const override { return PARAM_GROUP_COUNT; }

  char const* param_group_name(int i) const override {
    static std::array<char const*, PARAM_GROUP_COUNT> names = {
        "Global", "Field", "System",
    };
    return names[i];
  }

  //
  // PARAM
  //
  enum {
    PARAM_TIME,
    PARAM_TIME_LIMIT,
    PARAM_ALPHA,
    PARAM_GAIN,
    PARAM_BIAS,
    PARAM_AMP0,
    PARAM_AMP1,
    PARAM_AMP2,
    PARAM_AMP3,
    PARAM_AMP4,
    PARAM_SEED,
    PARAM_COUNT,
  };

  int param_count() const override { return PARAM_COUNT; }

  ParamPrototype const* param_prototype(int i) const override {
    static std::array<ParamPrototype, PARAM_COUNT> const params = {
        ParamPrototype{"time", PARAM_GROUP_GLOBAL, 0, 0, 1500},
        ParamPrototype{"time_limit", PARAM_GROUP_GLOBAL, 8, 2, 250},
        ParamPrototype{"alpha", PARAM_GROUP_GLOBAL, 0.80, 0, 1},
        ParamPrototype{"gain", PARAM_GROUP_GLOBAL, 1, 0, 1},
        ParamPrototype{"bias", PARAM_GROUP_GLOBAL, 0.5, 0, 1},
        ParamPrototype{"amp0", PARAM_GROUP_FIELD, 1.0, 0, 1},
        ParamPrototype{"amp1", PARAM_GROUP_FIELD, 0.8, 0, 1},
        ParamPrototype{"amp2", PARAM_GROUP_FIELD, 0.6, 0, 1},
        ParamPrototype{"amp3", PARAM_GROUP_FIELD, 0.4, 0, 1},
        ParamPrototype{"amp4", PARAM_GROUP_FIELD, 0.2, 0, 1},
        ParamPrototype{"seed", PARAM_GROUP_SYSTEM, 0.5, 0.0, 1},
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

  int compute(Config const& config, Params const& params, Args const& args,
              cv::Mat& retimg) override try {
    DEBUG_PRINT(__FUNCTION__);

    cv::Size const size = retimg.size();

    //
    // Params
    //
    int const time = params.get<int>(PARAM_TIME);
    int const time_limit = params.get<int>(PARAM_TIME_LIMIT) - 1;
    float const alpha = params.get<float>(PARAM_ALPHA);
    float const gain = params.get<float>(PARAM_GAIN);
    float const bias = params.get<float>(PARAM_BIAS);
    std::array<float, 5> const amp = {
        params.get<float>(PARAM_AMP0), params.get<float>(PARAM_AMP1),
        params.get<float>(PARAM_AMP2), params.get<float>(PARAM_AMP3),
        params.get<float>(PARAM_AMP4),
    };
    cv::theRNG().state =
        params.seed<std::uint64_t>(PARAM_SEED);  // sed seed to random generator

    int ntimes =
        (time ? time - 1 : std::max(1, config.frame)) % (time_limit * 2);
    if (ntimes >= time_limit) {
      ntimes = time_limit * 2 - ntimes;
    }
    ++ntimes;

    // generate time-Coherent perlin noise
    cv::Mat field = tnzu::make_perlin_noise<float>(size, amp);
    for (int t = 0; t <= ntimes; ++t) {
      cv::Mat next = tnzu::make_perlin_noise<float>(size, amp);
      field *= alpha;
      field += next * (1 - alpha);
    }
    field *= gain / 5;
    field += bias;

    for (int y = 0; y < size.height; ++y) {
      float* dst = retimg.ptr<float>(y);
      float const* src = field.ptr<float>(y);
      for (int x = 0; x < size.width; ++x) {
        dst[x] = src[x];
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
