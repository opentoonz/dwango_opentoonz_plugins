#define TNZU_DEFINE_INTERFACE
#define TNZU_ENABLE_USERDATA
#include <toonz_utility.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// resize for filter
cv::Mat resize(cv::Mat const& src, cv::Size const& max_size, float scale) {
  cv::Size size = src.size();
  size.width = std::min(std::max(static_cast<int>(size.width * scale), 1),
                        max_size.width);
  size.height = std::min(std::max(static_cast<int>(size.height * scale), 1),
                         max_size.height);

  cv::Mat tmp;
  cv::resize(src, tmp, size, 0.0, 0.0, cv::INTER_AREA);

  cv::Mat dst = cv::Mat::zeros(max_size, tmp.type());
  int const cx = size.width / 2;
  int const cy = size.height / 2;
  // | (a) (b) |to | (d) (c) |
  // | (c) (d) |   | (b) (a) |
  // (a)
  tmp(cv::Rect(0, 0, cx, cy))
      .copyTo(dst(cv::Rect(max_size.width - cx, max_size.height - cy, cx, cy)));

  // (b)
  tmp(cv::Rect(cx, 0, size.width - cx, cy))
      .copyTo(dst(cv::Rect(0, max_size.height - cy, size.width - cx, cy)));

  // (c)
  tmp(cv::Rect(0, cy, cx, size.height - cy))
      .copyTo(dst(cv::Rect(max_size.width - cx, 0, cx, size.height - cy)));

  // (d)
  tmp(cv::Rect(cx, cy, size.width - cx, size.height - cy))
      .copyTo(dst(cv::Rect(0, 0, size.width - cx, size.height - cy)));

  return dst;
}

// dft
cv::Mat dft(cv::Mat const& src, double const intensity = 1) {
  std::array<cv::Mat, 2> planes = {
      cv::Mat_<float>(src) * intensity, cv::Mat::zeros(src.size(), CV_32F),
  };

  cv::Mat dst;
  cv::merge(planes.data(), 2, dst);
  cv::dft(dst, dst);

  return dst;
}

class MyFx : public tnzu::Fx {
 public:
  //
  // PORT
  //
  enum {
    PORT_INPUT,
    PORT_A,
    PORT_B,
    PORT_C,
    PORT_COUNT,
  };

  int port_count() const override { return PORT_COUNT; }

  char const* port_name(int i) const override {
    static std::array<char const*, PORT_COUNT> names = {
        "Input", "A", "B", "C",
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
    PARAM_INTENSITY_A,
    PARAM_INTENSITY_B,
    PARAM_INTENSITY_C,
    PARAM_SCALE_A,
    PARAM_SCALE_B,
    PARAM_SCALE_C,
    PARAM_MARGIN,
    PARAM_COUNT,
  };

  int param_count() const override { return PARAM_COUNT; }

  ParamPrototype const* param_prototype(int i) const override {
    static std::array<ParamPrototype, PARAM_COUNT> const params = {
        ParamPrototype{"intensity_a", PARAM_GROUP_DEFAULT, 1, 0, 8},
        ParamPrototype{"intensity_b", PARAM_GROUP_DEFAULT, 1, 0, 8},
        ParamPrototype{"intensity_c", PARAM_GROUP_DEFAULT, 1, 0, 8},
        ParamPrototype{"scale_a", PARAM_GROUP_DEFAULT, 1, 0, 8},
        ParamPrototype{"scale_b", PARAM_GROUP_DEFAULT, 1, 0, 8},
        ParamPrototype{"scale_c", PARAM_GROUP_DEFAULT, 1, 0, 8},
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
    retrc.width += margin * 2;
    retrc.height += margin * 2;
    return 0;
  }

  int compute(Config const& config, Params const& params, Args const& args,
              cv::Mat& retimg) override try {
    DEBUG_PRINT(__FUNCTION__);

    if (args.invalid(PORT_INPUT)) {
      return 0;
    }

    int const max_value = (retimg.type() == CV_8UC4)
                              ? std::numeric_limits<uchar>::max()
                              : std::numeric_limits<ushort>::max();

    int const margin = params.get<int>(PARAM_MARGIN);
    float const intensity_a = params.get<float>(PARAM_INTENSITY_A);
    float const intensity_b = params.get<float>(PARAM_INTENSITY_B);
    float const intensity_c = params.get<float>(PARAM_INTENSITY_C);
    float const scale_a = params.get<float>(PARAM_SCALE_A);
    float const scale_b = params.get<float>(PARAM_SCALE_B);
    float const scale_c = params.get<float>(PARAM_SCALE_C);
    bool const input_a = args.valid(PORT_A);
    bool const input_b = args.valid(PORT_B);
    bool const input_c = args.valid(PORT_C);

    cv::Size const isize = args.size(PORT_INPUT);
    cv::Size const msize(isize.width + 2 * margin, isize.height + 2 * margin);
    cv::Size const osize(cv::getOptimalDFTSize(msize.width),
                         cv::getOptimalDFTSize(msize.height));

    std::array<cv::Mat, 4> I;
    cv::split(args.get(PORT_INPUT), I.data());

    std::array<cv::Mat, 4> A;
    double ka = 1;
    if (input_a) {
      cv::Mat a = resize(args.get(PORT_A), osize, scale_a);
      cv::Mat g;
      cv::cvtColor(a, g, cv::COLOR_BGRA2GRAY);
      cv::split(g, A.data());
      double const sum = cv::sum(cv::Mat_<float>(A[0]))[0];
      if (sum > 0.0) {
        ka /= sum;
      }
      cv::split(a, A.data());
    }

    std::array<cv::Mat, 4> B;
    double kb = 1;
    if (input_b) {
      cv::Mat b = resize(args.get(PORT_B), osize, scale_b);
      cv::Mat g;
      cv::cvtColor(b, g, cv::COLOR_BGRA2GRAY);
      cv::split(g, B.data());
      double const sum = cv::sum(cv::Mat_<float>(B[0]))[0];
      if (sum > 0.0) {
        kb /= sum;
      }
      cv::split(b, B.data());
    }

    std::array<cv::Mat, 4> C;
    double kc = 1;
    if (input_c) {
      cv::Mat c = resize(args.get(PORT_C), osize, scale_c);
      cv::Mat g;
      cv::cvtColor(c, g, cv::COLOR_BGRA2GRAY);
      cv::split(g, C.data());
      double const sum = cv::sum(cv::Mat_<float>(C[0]))[0];
      if (sum > 0.0) {
        kc /= sum;
      }
      cv::split(c, C.data());
    }

    // donot use filter2D to apply dft only once for each Mat
    std::array<cv::Mat, 4> R;
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (int c = 0; c < 4; ++c) {
      cv::Mat paddedI;
      cv::copyMakeBorder(I[c], paddedI, margin,
                         osize.height - (isize.height + margin), margin,
                         osize.width - (isize.width + margin),
                         cv::BORDER_CONSTANT, cv::Scalar::all(0));

      cv::Mat complexI = dft(paddedI);
      if (input_a) {
        cv::Mat complexA = dft(A[c], ((c == 3) ? 1 : intensity_a) * ka);
        cv::mulSpectrums(complexI, complexA, complexI, 0);
      }
      if (input_b) {
        cv::Mat complexB = dft(B[c], ((c == 3) ? 1 : intensity_b) * kb);
        cv::mulSpectrums(complexI, complexB, complexI, 0);
      }
      if (input_c) {
        cv::Mat complexC = dft(C[c], ((c == 3) ? 1 : intensity_c) * kc);
        cv::mulSpectrums(complexI, complexC, complexI, 0);
      }

      cv::idft(complexI, complexI, cv::DFT_SCALE);

      std::array<cv::Mat, 2> planes;
      cv::split(complexI, planes.data());

      planes[0] = planes[0](cv::Rect(cv::Point(0, 0), msize));
      if (retimg.type() == CV_8UC4) {
        using value_type = uchar;

        planes[0].forEach<float>([max_value](float& value, void const*) {
          if (value > 0.0f) {
            value = tnzu::normalize_cast<value_type>(value / max_value);
          }
        });

        R[c] = cv::Mat_<value_type>(planes[0]);
      } else {
        using value_type = ushort;

        planes[0].forEach<float>([max_value](float& value, void const*) {
          if (value > 0.0f) {
            value = tnzu::normalize_cast<value_type>(value / max_value);
          }
        });

        R[c] = cv::Mat_<value_type>(planes[0]);
      }
    }

    cv::Mat dst;
    cv::merge(R.data(), R.size(), dst);
    cv::Point const offset(
        static_cast<int>(args.offset(PORT_INPUT).x - margin),
        static_cast<int>(args.offset(PORT_INPUT).y - margin));
    dst.copyTo(retimg(cv::Rect(offset, msize)));

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
