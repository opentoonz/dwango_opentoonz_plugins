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
    PARAM_GAMMA,
    PARAM_EXPOSURE,
    PARAM_COUNT,
  };

  int param_count() const override { return PARAM_COUNT; }

  ParamPrototype const* param_prototype(int i) const override {
    static std::array<ParamPrototype, PARAM_COUNT> const params = {
        ParamPrototype{"gamma", PARAM_GROUP_DEFAULT, 2.2, 0.100, 5.0},
        ParamPrototype{"exposure", PARAM_GROUP_DEFAULT, 1.0, 0.125, 8.0},
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

  float const gamma = params.get<float>(PARAM_GAMMA);
  float const exposure = params.get<float>(PARAM_EXPOSURE);

  cv::Mat r(retimg.size(), CV_32FC3, cv::Scalar(0, 0, 0));
  cv::Mat t(retimg.size(), CV_32FC3, cv::Scalar(1, 1, 1));

  tnzu::linear_color_space_converter<sizeof(value_type) * 8> converter(exposure,
                                                                       gamma);
  float const alpha_scale = 1.0f / std::numeric_limits<value_type>::max();

  for (int i = args.count(); i-- > 0;) {
    if (args.invalid(i)) {
      continue;
    }

    cv::Size const size = args.size(i);

    cv::Mat rlocal(r, args.rect(i));
    cv::Mat tlocal(t, args.rect(i));

#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (int y = 0; y < size.height; y++) {
      Vec4T const* data = args.get(i).ptr<Vec4T const>(y);
      cv::Vec3f* rdata = rlocal.ptr<cv::Vec3f>(y);
      cv::Vec3f* tdata = tlocal.ptr<cv::Vec3f>(y);

      for (int x = 0; x < size.width; x++) {
        // linear sRGB (straight alpha)
        cv::Vec3f const bgr(converter[data[x][0]],   // blue
                            converter[data[x][1]],   // green
                            converter[data[x][2]]);  // red
        float const alpha = data[x][3] * alpha_scale;

        // XYZ color space (D65)
        cv::Vec3f const xyz = tnzu::to_xyz(bgr);

        // Composite
        for (int c = 0; c < 3; c++) {
          float const R1 = rdata[x][c];
          float const T1 = tdata[x][c];
          float const R2 = xyz[c];
          float const T2 = 1.0f - alpha;
          float const id = 1.0f / (1.0f - R1 * R2);

          rdata[x][c] = R1 + T1 * T1 * R2 * id;
          tdata[x][c] = T1 * T2 * id;
        }
      }
    }
  }

  cv::Size const size = retimg.size();
#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int y = 0; y < size.height; y++) {
    Vec4T* data = retimg.ptr<Vec4T>(y);
    cv::Vec3f const* rdata = r.ptr<cv::Vec3f const>(y);
    cv::Vec3f const* tdata = t.ptr<cv::Vec3f const>(y);

    for (int x = 0; x < size.width; x++) {
      // XYZ color space: {X, Y, Z}
      cv::Vec3f const xyz(rdata[x][0], rdata[x][1], rdata[x][2]);
      float const alpha = 1.0f - tdata[x][1];

      // linear RGB
      cv::Vec3f const bgr = tnzu::to_bgr(xyz);

      // sRGB (straight alpha)
      cv::Vec4f const sbgra(
          tnzu::to_nonlinear_color_space(bgr[0], exposure, gamma),
          tnzu::to_nonlinear_color_space(bgr[1], exposure, gamma),
          tnzu::to_nonlinear_color_space(bgr[2], exposure, gamma), alpha);

      for (int c = 0; c < 4; c++) {
        data[x][c] = tnzu::normalize_cast<value_type>(sbgra[c]);
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
