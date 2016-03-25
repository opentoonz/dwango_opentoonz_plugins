#define TNZU_DEFINE_INTERFACE
#include <toonz_utility.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class MyFx : public tnzu::Fx {
 public:
  //
  // PORT
  //
  enum {
    PORT_INPUT_0,
    PORT_INPUT_1,
    PORT_INPUT_2,
    PORT_INPUT_3,
    PORT_INPUT_4,
    PORT_INPUT_5,
    PORT_INPUT_6,
    PORT_INPUT_7,
    PORT_INPUT_8,
    PORT_INPUT_9,
    PORT_COUNT,
  };

  int port_count() const override { return PORT_COUNT; }

  char const* port_name(int i) const override {
    static std::array<char const*, PORT_COUNT> names = {
        "layer0", "layer1", "layer2", "layer3", "layer4",
        "layer5", "layer6", "layer7", "layer8", "layer9",
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
    PARAM_COUNT,
  };

  int param_count() const override { return PARAM_COUNT; }

  ParamPrototype const* param_prototype(int i) const override {
    return nullptr;
  }

 public:
  template <typename Vec4T>
  int kernel(Config const& config, Params const& params, Args const& args,
             cv::Mat& retimg);

  int compute(Config const& config, Params const& params, Args const& args,
              cv::Mat& retimg) override try {
    DEBUG_PRINT(__FUNCTION__);
    if (retimg.type() == CV_8UC4) {
      return kernel<cv::Vec4b>(config, params, args, retimg);
    } else {
      return kernel<cv::Vec4w>(config, params, args, retimg);
    }
  } catch (cv::Exception const& e) {
    DEBUG_PRINT(e.what());
  }
};

template <typename Vec4T>
int MyFx::kernel(Config const& config, Params const& params, Args const& args,
                 cv::Mat& retimg) {
  using value_type = typename Vec4T::value_type;

  float const gamma = 2.2f;

  cv::Mat accum(retimg.size(), CV_32FC4, cv::Scalar(0, 0, 0, 0));

  tnzu::linear_color_space_converter<sizeof(value_type) * 8> converter(1.0f,
                                                                       gamma);
  float const alpha_scale = 1.0f / std::numeric_limits<value_type>::max();

  for (int i = 0, argc = args.count(); i < argc; ++i) {
    if (args.invalid(i)) {
      continue;
    }

    cv::Size const size = args.size(i);

    cv::Mat local_view(accum, args.rect(i));

#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (int y = 0; y < size.height; y++) {
      Vec4T const* src = args.get(i).ptr<Vec4T const>(y);
      cv::Vec4f* dst = local_view.ptr<cv::Vec4f>(y);

      for (int x = 0; x < size.width; x++) {
        // linear sRGB (straight alpha)
        cv::Vec3f const bgr(converter[src[x][0]],   // blue
                            converter[src[x][1]],   // green
                            converter[src[x][2]]);  // red
        float const alpha = src[x][3] * alpha_scale;

        // XYZ color space (D65)
        cv::Vec3f const xyz = tnzu::to_xyz(bgr);

        dst[x][0] += xyz[0];
        dst[x][1] += xyz[1];
        dst[x][2] += xyz[2];
        dst[x][3] += alpha;
      }
    }
  }

  cv::Size const size = retimg.size();
#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int y = 0; y < size.height; y++) {
    Vec4T* dst = retimg.ptr<Vec4T>(y);
    cv::Vec4f const* src = accum.ptr<cv::Vec4f const>(y);

    for (int x = 0; x < size.width; x++) {
      // XYZ color space: {X, Y, Z}
      cv::Vec3f const xyz(src[x][0], src[x][1], src[x][2]);

      // linear RGB
      cv::Vec3f const bgr = tnzu::to_bgr(xyz);

      // sRGB (straight alpha)
      cv::Vec4f const color(tnzu::to_nonlinear_color_space(bgr[0], 1.0f, gamma),
                            tnzu::to_nonlinear_color_space(bgr[1], 1.0f, gamma),
                            tnzu::to_nonlinear_color_space(bgr[2], 1.0f, gamma),
                            src[x][3]);

      for (int c = 0; c < 4; c++) {
        dst[x][c] = tnzu::normalize_cast<value_type>(color[c]);
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
