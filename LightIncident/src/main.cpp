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
    PARAM_GROUP_TIME,
    PARAM_GROUP_GEOMETRY,
    PARAM_GROUP_COLOR,
    PARAM_GROUP_FIGURE,
    PARAM_GROUP_FALLOFF,
    PARAM_GROUP_DISPERSION,
    PARAM_GROUP_BLOOM,
    PARAM_GROUP_SYSTEM,
    PARAM_GROUP_COUNT,
  };

  int param_group_count() const override { return PARAM_GROUP_COUNT; }

  char const* param_group_name(int i) const override {
    static std::array<char const*, PARAM_GROUP_COUNT> names = {
        "time",    "geometry",   "color", "figure",
        "falloff", "dispersion", "bloom", "system",
    };
    return names[i];
  }

  //
  // PARAM
  //
  enum {
    // time
    PARAM_TIME_TIME,
    PARAM_TIME_TIME_LIMIT,
    PARAM_TIME_BETA,
    PARAM_TIME_GAMMA,

    // geometry
    PARAM_GEOMETRY_DISTANCE,
    PARAM_GEOMETRY_THETA,
    PARAM_GEOMETRY_PHI,
    PARAM_GEOMETRY_ALPHA,
    PARAM_GEOMETRY_WIDTH,
    PARAM_GEOMETRY_LENGTH,
    PARAM_GEOMETRY_SCRAGGLY,
    PARAM_GEOMETRY_ROUGHNESS,
    PARAM_GEOMETRY_DISTINCTNESS,
    PARAM_GEOMETRY_NUMBER,

    // color
    PARAM_COLOR_R,
    PARAM_COLOR_G,
    PARAM_COLOR_B,
    PARAM_COLOR_INTENSITY,

    // figure
    PARAM_FIGURE_BLUR,

    // falloff
    PARAM_FALLOFF_DISTANCE,
    PARAM_FALLOFF_SENSITIVITY,

    // dispersion
    PARAM_DISPERSION_RATE,
    PARAM_DISPERSION_BIAS,
    PARAM_DISPERSION_GAIN,

    // bloom
    PARAM_BLOOM_LEVEL,
    PARAM_BLOOM_GAIN,
    PARAM_BLOOM_BIAS,

    // system
    PARAM_SEED_INTENSITY,
    PARAM_SEED_DIRECTION,
    PARAM_SEED_WIDTH,
    PARAM_SEED_LENGTH,
    PARAM_SEED_GAMMA,
    PARAM_SEED_PHASE,
    PARAM_COUNT,
  };

  int param_count() const override { return PARAM_COUNT; }

  ParamPrototype const* param_prototype(int i) const override {
    static std::array<ParamPrototype, PARAM_COUNT> const params = {
        ParamPrototype{"time", PARAM_GROUP_TIME, 0, 0, 1500},
        ParamPrototype{"time_limit", PARAM_GROUP_TIME, 8, 2, 250},
        ParamPrototype{"beta", PARAM_GROUP_TIME, 10, 0, 30},
        ParamPrototype{"gamma", PARAM_GROUP_TIME, 0.001, 0, 1},

        // geometry
        ParamPrototype{"distance", PARAM_GROUP_GEOMETRY, 2.00, 0.00, 5.0},
        ParamPrototype{"theta", PARAM_GROUP_GEOMETRY, 40.00, -180.00, 180.0},
        ParamPrototype{"phi", PARAM_GROUP_GEOMETRY, 30.00, 0.00, 90.0},
        ParamPrototype{"alpha", PARAM_GROUP_GEOMETRY, 0.00, -45.00, 45.0},
        ParamPrototype{"width", PARAM_GROUP_GEOMETRY, 0.10, 0.00, 30.0},
        ParamPrototype{"length", PARAM_GROUP_GEOMETRY, 2.00, 0.01, 10.0},
        ParamPrototype{"scraggly", PARAM_GROUP_GEOMETRY, 0.20, 0.01, 2.0},
        ParamPrototype{"roughness", PARAM_GROUP_GEOMETRY, 0.02, 0.01, 1.0},
        ParamPrototype{"distinctness", PARAM_GROUP_GEOMETRY, 0.50, 0.01, 2.0},
        ParamPrototype{"number", PARAM_GROUP_GEOMETRY, 40.00, 1.00, 100.0},

        // color
        ParamPrototype{"r", PARAM_GROUP_COLOR, 1.0, 0.0, 1.0},
        ParamPrototype{"g", PARAM_GROUP_COLOR, 1.0, 0.0, 1.0},
        ParamPrototype{"b", PARAM_GROUP_COLOR, 1.0, 0.0, 1.0},
        ParamPrototype{"intensity", PARAM_GROUP_COLOR, 2.0, 0.01, 100.0},

        // figure
        ParamPrototype{"blur", PARAM_GROUP_FIGURE, 0.01, 0, 0.5},

        // falloff
        ParamPrototype{"falloff", PARAM_GROUP_FALLOFF, 0.8, 0.00, 1.0},
        ParamPrototype{"sensitivity", PARAM_GROUP_FALLOFF, 0.1, 0.01, 2.0},

        // dispersion
        ParamPrototype{"d_rate", PARAM_GROUP_DISPERSION, 7.0, 0.0, 8.0},
        ParamPrototype{"d_bias", PARAM_GROUP_DISPERSION, 0.5, 0.0, 8.0},
        ParamPrototype{"d_gain", PARAM_GROUP_DISPERSION, 0.3, 0.0, 1.0},

        // bloom
        ParamPrototype{"level", PARAM_GROUP_BLOOM, 6.0, 0.0, 32.0},
        ParamPrototype{"gain", PARAM_GROUP_BLOOM, 1.0, 0.0, 8.0},
        ParamPrototype{"bias", PARAM_GROUP_BLOOM, 0.0, 0.0, 8.0},

        // system
        ParamPrototype{"seed_intensity", PARAM_GROUP_SYSTEM, 0.99, 0.0, 1.0},
        ParamPrototype{"seed_direction", PARAM_GROUP_SYSTEM, 0.98, 0.0, 1.0},
        ParamPrototype{"seed_width", PARAM_GROUP_SYSTEM, 0.97, 0.0, 1.0},
        ParamPrototype{"seed_length", PARAM_GROUP_SYSTEM, 0.96, 0.0, 1.0},
        ParamPrototype{"seed_gamma", PARAM_GROUP_SYSTEM, 0.95, 0.0, 1.0},
        ParamPrototype{"seed_phase", PARAM_GROUP_SYSTEM, 0.94, 0.0, 1.0},
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
  int kernel(Config const& config, Params const& params, Args const& args,
             cv::Mat& retimg);

  int compute(Config const& config, Params const& params, Args const& args,
              cv::Mat& retimg) override try {
    DEBUG_PRINT(__FUNCTION__);

    if (args.invalid(PORT_INPUT) || args.invalid(PORT_NOISE)) {
      return 0;
    }

    if (retimg.type() == CV_8UC4) {
      return kernel<cv::Vec4b>(config, params, args, retimg);
    } else {
      return kernel<cv::Vec4w>(config, params, args, retimg);
    }

    return 0;
  } catch (cv::Exception const& e) {
    DEBUG_PRINT(e.what());
  }
};

template <typename Vec4T>
int MyFx::kernel(Config const& config, Params const& params, Args const& args,
                 cv::Mat& retimg) {
  using value_type = typename Vec4T::value_type;

  cv::Size const size = retimg.size();

  //
  // Params
  //
  // time
  int const time = params.get<int>(PARAM_TIME_TIME);
  int const time_limit = params.get<int>(PARAM_TIME_TIME_LIMIT) - 1;
  float const beta = params.get<float>(PARAM_TIME_BETA);
  float const gamma = params.radian<float>(PARAM_TIME_GAMMA);

  // geometry
  float const distance =
      params.get<float>(PARAM_GEOMETRY_DISTANCE, size.height);
  float const theta = params.radian<float>(PARAM_GEOMETRY_THETA);
  float const phi = params.radian<float>(PARAM_GEOMETRY_PHI);
  float const alpha = params.radian<float>(PARAM_GEOMETRY_ALPHA);
  float const width = params.radian<float>(PARAM_GEOMETRY_WIDTH);
  float const length = params.get<float>(PARAM_GEOMETRY_LENGTH);
  float const scraggly = params.get<float>(PARAM_GEOMETRY_SCRAGGLY);
  float const roughness =
      params.get<float>(PARAM_GEOMETRY_ROUGHNESS, size.height);
  float const distinctness = params.get<float>(PARAM_GEOMETRY_DISTINCTNESS);
  int const number = params.get<int>(PARAM_GEOMETRY_NUMBER);

  // color
  cv::Scalar const color(params.get<double>(PARAM_COLOR_B),
                         params.get<double>(PARAM_COLOR_G),
                         params.get<double>(PARAM_COLOR_R));
  float const intensity = params.get<float>(PARAM_COLOR_INTENSITY);

  // figure
  int const blur = params.get<int>(PARAM_FIGURE_BLUR, size.height) * 2 + 1;

  // falloff
  float const falloff = params.get<float>(PARAM_FALLOFF_DISTANCE);
  float const sensitivity =
      params.get<float>(PARAM_FALLOFF_SENSITIVITY, size.height);

  // dispersion
  float const drate = params.get<float>(PARAM_DISPERSION_RATE, M_PI * 2);
  float const dbias = params.get<float>(PARAM_DISPERSION_BIAS);
  float const dgain = params.get<float>(PARAM_DISPERSION_GAIN, dbias);

  // bloom
  int const bloom = params.get<int>(PARAM_BLOOM_LEVEL);
  float const gain = params.get<float>(PARAM_BLOOM_GAIN);
  float const bias = params.get<float>(PARAM_BLOOM_BIAS);

  // system
  std::mt19937_64 engine_intensity =
      params.rng<std::uint64_t>(PARAM_SEED_INTENSITY);
  std::mt19937_64 engine_direction =
      params.rng<std::uint64_t>(PARAM_SEED_DIRECTION);
  std::mt19937_64 engine_width = params.rng<std::uint64_t>(PARAM_SEED_WIDTH);
  std::mt19937_64 engine_length = params.rng<std::uint64_t>(PARAM_SEED_LENGTH);
  std::mt19937_64 engine_gamma = params.rng<std::uint64_t>(PARAM_SEED_GAMMA);
  std::mt19937_64 engine_phase = params.rng<std::uint64_t>(PARAM_SEED_PHASE);

  // draw a background
  tnzu::draw_image(retimg, args.get(PORT_INPUT), args.offset(PORT_INPUT));

  // make a mask image
  int const g = std::numeric_limits<value_type>::max();
  cv::Mat mask(size, retimg.type(), cv::Scalar(g, g, g, g));
  if (args.valid(PORT_MASK)) {
    tnzu::draw_image(mask, args.get(PORT_MASK), args.offset(PORT_MASK));
  }

  // define light bar
  cv::Mat light = cv::Mat::zeros(size, CV_32FC3);

  // intensity field
  cv::Mat const field = args.get(PORT_NOISE);

  // draw light bar
  cv::Point2f const org(size.width / 2 + distance * std::cos(theta),
                        size.height / 2 + distance * std::sin(theta));
  {
    std::uniform_real_distribution<float> runif_angle(theta - phi * 0.5f,
                                                      theta + phi * 0.5f);
    std::uniform_real_distribution<float> runif_width(0.0f, width);
    std::uniform_real_distribution<float> runif_gamma(-gamma, gamma);
    std::uniform_real_distribution<float> runif_intensity(0.0f, intensity);
    std::cauchy_distribution<float> rcauchy_length(length, scraggly);
    std::uniform_real_distribution<float> runif_phase;

    int ntimes =
        (time ? time - 1 : std::max(1, config.frame)) % (time_limit * 2);
    if (ntimes >= time_limit) {
      ntimes = time_limit * 2 - ntimes;
    }
    ++ntimes;

    std::vector<float> lengths(number);
    std::vector<float> intensities(number);
    for (int i = 0; i < number; i++) {
      intensities[i] = runif_intensity(engine_intensity);
    }
    std::sort(intensities.begin(), intensities.end());
    for (int i = 0; i < number; i++) {
      float angle = runif_angle(engine_direction);
      for (int t = 0; t <= ntimes; ++t) {
        angle += runif_gamma(engine_gamma) * beta;
      }
      angle += alpha;
      float const delta = runif_width(engine_width);
      float const len =
          std::min(std::max(rcauchy_length(engine_length) * size.height, 0.01f),
                   length * size.height * 2.0f);
      cv::Scalar const c = color * intensities[i];
      float const g = tnzu::to_gray(c);

      float const diameter = roughness * distinctness;
      float const height = len * std::tan(delta);
      cv::Point2f const dir(std::cos(angle), std::sin(angle));
      for (int n = 0, count = static_cast<int>(len / roughness); n < count;
           ++n) {
        float const t = (n + runif_phase(engine_phase)) / count;
        float const f =
            (t < falloff) ? 1.0f : std::exp((falloff - t) * sensitivity);

        cv::Point2f const center = org - dir * (t * len);
        cv::Size2f const radius(diameter, t * height * g);

        cv::ellipse(light, center, radius, tnzu::to_degree(angle), 0, 360,
                    c * f, -1);
      }
    }
  }

  // blur bars
  cv::GaussianBlur(light, light, cv::Size(blur, blur), 0.0);

  {
    float const dr = float(2 * M_PI) / 0.6850f;
    float const dg = float(2 * M_PI) / 0.5325f;
    float const db = float(2 * M_PI) / 0.4725f;

#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (int y = 0; y < size.height; y++) {
      float const* fld = field.ptr<float>(y);
      Vec4T const* msk = mask.ptr<Vec4T>(y);
      cv::Vec3f* dst = light.ptr<cv::Vec3f>(y);
      for (int x = 0; x < size.width; x++) {
        float const intensity = std::max(fld[x] * gain + bias, 0.01f) /
                                std::numeric_limits<value_type>::max();

        float const dx = (x - org.x) / size.height;
        float const dy = (y - org.y) / size.height;
        float const d2 = dx * dx + dy * dy;
        float const d = std::sqrt(d2);
        float const depth = std::cos(d * drate) * 0.5f + 0.5f;
        float const cr = dbias + dgain * std::cos(dr * depth);
        float const cg = dbias + dgain * std::cos(dg * depth);
        float const cb = dbias + dgain * std::cos(db * depth);

        dst[x][0] *= intensity * cb * msk[x][0];
        dst[x][1] *= intensity * cg * msk[x][1];
        dst[x][2] *= intensity * cr * msk[x][2];
      }
    }
  }

  // generate bloom
  cv::GaussianBlur(light, light, cv::Size(blur, blur), 0.0);
  tnzu::generate_bloom(light, bloom);

  // init color table
  tnzu::linear_color_space_converter<sizeof(value_type) * 8> converter(1.0f,
                                                                       2.2f);

// add incident light on linear color space
#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int y = 0; y < size.height; y++) {
    cv::Vec3f const* s = light.ptr<cv::Vec3f>(y);
    Vec4T* d = retimg.ptr<Vec4T>(y);
    for (int x = 0; x < size.width; x++) {
      cv::Vec4f const sbgra(tnzu::to_nonlinear_color_space(
                                converter[d[x][0]] + s[x][0], 1.0f, 2.2f),
                            tnzu::to_nonlinear_color_space(
                                converter[d[x][1]] + s[x][1], 1.0f, 2.2f),
                            tnzu::to_nonlinear_color_space(
                                converter[d[x][2]] + s[x][2], 1.0f, 2.2f),
                            1.0f);
      for (int c = 0; c < 4; ++c) {
        d[x][c] = tnzu::normalize_cast<value_type>(sbgra[c]);
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
