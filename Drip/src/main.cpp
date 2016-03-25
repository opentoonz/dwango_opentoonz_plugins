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
    PARAM_THRESHOLD,
    PARAM_COUNT,
  };

  int param_count() const override { return PARAM_COUNT; }

  ParamPrototype const* param_prototype(int i) const override {
    static std::array<ParamPrototype, PARAM_COUNT> const params = {
        ParamPrototype{"threshold", 0, 0.9, 0, 1},
    };
    return &params[i];
  }

 public:
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
  value_type const max_value = std::numeric_limits<value_type>::max();

  // init parameters
  int const t = params.get<int>(PARAM_THRESHOLD, (299 + 587 + 114) * max_value);

  args.get(PORT_INPUT).copyTo(retimg(args.rect(PORT_INPUT)));

  retimg.forEach<Vec4T>([t](Vec4T& p, void const*) {
    int const g = 299 * p[2] + 587 * p[1] + 114 * p[0];
    if (g < t) {
      p = Vec4T(0, 0, 0, 0);
    }
  });
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
