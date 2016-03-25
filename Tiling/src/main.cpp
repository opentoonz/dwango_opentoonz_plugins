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
    PARAM_MIRRORING,
    PARAM_COUNT,
  };

  int param_count() const override { return PARAM_COUNT; }

  ParamPrototype const* param_prototype(int i) const override {
    static std::array<ParamPrototype, PARAM_COUNT> const params = {
        ParamPrototype{"mirroring", PARAM_GROUP_DEFAULT, 0, 0, 1},
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
  bool const mirroring = params.get<bool>(PARAM_MIRRORING);

  cv::Size const dsize = retimg.size();
  cv::Size const ssize = args.size(PORT_INPUT);
  if ((ssize.width <= 0) || (ssize.height <= 0)) {
    return 0;
  }

  cv::Point const offset(
      ssize.width -
          std::abs(static_cast<int>(args.offset(PORT_INPUT).x) % ssize.width),
      ssize.height -
          std::abs(static_cast<int>(args.offset(PORT_INPUT).y) % ssize.height));
  if (mirroring) {
    for (int y = 0; y < dsize.height; y++) {
      int sy = (y + offset.y) % (2 * ssize.height);
      if (sy >= ssize.height) {
        sy = 2 * ssize.height - 1 - sy;
      }

      Vec4T const* s = args.get(PORT_INPUT).ptr<Vec4T>(sy);
      Vec4T* d = retimg.ptr<Vec4T>(y);
      for (int x = 0; x < dsize.width; x++) {
        int sx = (x + offset.x) % (2 * ssize.width);
        if (sx >= ssize.width) {
          sx = 2 * ssize.width - 1 - sx;
        }

        d[x] = s[sx];
      }
    }
  } else {
    for (int y = 0; y < dsize.height; y++) {
      int const sy = (y + offset.y) % ssize.height;
      Vec4T const* s = args.get(PORT_INPUT).ptr<Vec4T>(sy);
      Vec4T* d = retimg.ptr<Vec4T>(y);
      for (int x = 0; x < dsize.width; x++) {
        int const sx = (x + offset.x) % ssize.width;
        d[x] = s[sx];
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
