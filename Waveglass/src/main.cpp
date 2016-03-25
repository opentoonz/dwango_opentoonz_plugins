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
  // GROUP
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
    PARAM_ETA,
    PARAM_HEIGHT,
    PARAM_DEPTH,
    PARAM_RED,
    PARAM_GREEN,
    PARAM_BLUE,
    PARAM_BLUR,
    PARAM_COUNT,
  };

  int param_count() const override { return PARAM_COUNT; }

  ParamPrototype const* param_prototype(int i) const override {
    static std::array<ParamPrototype, PARAM_COUNT> const params = {
        ParamPrototype{"gain", PARAM_GROUP_DEFAULT, 1.00, 0, 32},
        ParamPrototype{"eta", PARAM_GROUP_DEFAULT, 2.00, 1, 2.5},
        ParamPrototype{"height", PARAM_GROUP_DEFAULT, 1.00, 0, 32},
        ParamPrototype{"depth", PARAM_GROUP_DEFAULT, 1.00, 0, 2},
        ParamPrototype{"red", PARAM_GROUP_DEFAULT, 0.00, 0, 0.5},
        ParamPrototype{"green", PARAM_GROUP_DEFAULT, 0.00, 0, 0.5},
        ParamPrototype{"blue", PARAM_GROUP_DEFAULT, 0.00, 0, 0.5},
        ParamPrototype{"blur", PARAM_GROUP_DEFAULT, 0.01, 0, 1},
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

  template <typename Vec4T>
  int kernel(cv::Mat const& iimage, cv::Point2d ioffset, cv::Mat const& mimage,
             cv::Point2d moffset, cv::Mat const& field, float const gain,
             float const eta, float const height, float const depth,
             cv::Vec3f const& attenuation, cv::Mat& retimg);

  int compute(Config const& config, Params const& params, Args const& args,
              cv::Mat& retimg) override try {
    DEBUG_PRINT(__FUNCTION__);

    if (args.invalid(PORT_INPUT) || args.invalid(PORT_NOISE)) {
      return 0;
    }

    cv::Size const size = retimg.size();
    int const type = retimg.type();

    //
    // Params
    //
    float const gain = params.get<float>(PARAM_GAIN);
    float const eta = params.get<float>(PARAM_ETA);
    float const height = params.get<float>(PARAM_HEIGHT, size.height);
    float const depth = params.get<float>(PARAM_DEPTH);
    cv::Vec3f const attenuation(params.get<float>(PARAM_BLUE),
                                params.get<float>(PARAM_GREEN),
                                params.get<float>(PARAM_RED));
    int const blur = params.get<int>(PARAM_BLUR, size.height) * 2 + 1;

    // init input
    cv::Mat input = args.get(PORT_INPUT);
    cv::GaussianBlur(input, input, cv::Size(blur, blur), 0.0);

    // init noise
    cv::Mat field = args.get(PORT_NOISE);

    // init mask
    cv::Mat mask;
    cv::Point2d mask_offset;
    if (args.valid(PORT_MASK)) {
      mask = args.get(PORT_MASK);
      mask_offset = args.offset(PORT_MASK);
    } else {
      double const g = (type == CV_8UC4) ? std::numeric_limits<uchar>::max()
                                         : std::numeric_limits<ushort>::max();

      mask = cv::Mat(size, type, cv::Scalar(g, g, g, g));
      mask_offset = cv::Point2d(0, 0);
    }

    // apply waveglass
    if (type == CV_8UC4) {
      return kernel<cv::Vec4b>(input, args.offset(0), mask, mask_offset, field,
                               gain, eta, height, depth, attenuation, retimg);
    } else {
      return kernel<cv::Vec4w>(input, args.offset(0), mask, mask_offset, field,
                               gain, eta, height, depth, attenuation, retimg);
    }
  } catch (cv::Exception const& e) {
    DEBUG_PRINT(e.what());
  }
};

template <typename Vec4T>
int MyFx::kernel(cv::Mat const& iimage, cv::Point2d ioffset,
                 cv::Mat const& mimage, cv::Point2d moffset,
                 cv::Mat const& field, float const gain, float const eta,
                 float const height, float const depth,
                 cv::Vec3f const& attenuation, cv::Mat& retimg) {
  using value_type = typename Vec4T::value_type;

  float const max_value = std::numeric_limits<value_type>::max();
  float const scale = 1.0f / max_value;

  cv::Size const size = retimg.size();
  cv::Point3f const direction(0.0f, 0.0f, -1.0f);

  // init color table
  tnzu::linear_color_space_converter<sizeof(value_type) * 8> converter(1.0f,
                                                                       2.2f);

// apply a wave glass
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
      float const dx =
          (field.at<float>(y, x1) - field.at<float>(y, x0)) * (gain * 0.5f);
      float const dy =
          (field.at<float>(y1, x) - field.at<float>(y0, x)) * (gain * 0.5f);
      float const z = field.at<float>(y, x) * gain;

      // tap a mask image
      int const mx = static_cast<int>(x - moffset.x);
      int const my = static_cast<int>(y - moffset.y);
      float mask = 0;
      if ((0 <= mx) && (mx < mimage.cols) && (0 <= my) && (my < mimage.rows)) {
        mask = mimage.at<Vec4T>(my, mx)[3] * scale;
      }

      // compute refract vector
      cv::Point3f const bivector(1.0f, 0.0f, dx);
      cv::Point3f const binormal(0.0f, 1.0f, dy);
      cv::Point3f normal = bivector.cross(binormal);
      normal /= cv::norm(normal);

      cv::Point3f rvec = tnzu::refract(direction, normal, eta);
      if (rvec.z > -1e-8f) {
        DEBUG_PRINT("fatal error");
      }
      cv::Point3f const vec = rvec * (height / -rvec.z);

      float const tx = x + mask * vec.x;
      float const ty = y + mask * vec.y;

      // tap an input image
      cv::Point2d const tap_pos(tx - ioffset.x, ty - ioffset.y);

      Vec4T input;
      if ((0 <= tap_pos.x) && (tap_pos.x < iimage.cols) && (0 <= tap_pos.y) &&
          (tap_pos.y < iimage.rows)) {
        float const t = cv::norm(rvec * (std::abs(z + depth) / -rvec.z));

        Vec4T color = tnzu::tap_texel<Vec4T>(iimage, tap_pos);
        for (int c = 0; c < 3; ++c) {
          float const value =
              converter[color[c]] * std::exp(-attenuation[c] * t);
          color[c] = tnzu::normalize_cast<value_type>(
              tnzu::to_nonlinear_color_space(value, 1.0f, 2.2f));
        }
        input = color;
      } else {
        input = Vec4T(0, 0, 0, 0);
      }

      dst[x] = input;
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
