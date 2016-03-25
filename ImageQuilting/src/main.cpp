#define TNZU_DEFINE_INTERFACE
#include <toonz_utility.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class MyFx : public tnzu::Fx {
 public:
  //
  // PORT
  //
  enum {
    PORT_BACKGROUND,
    PORT_FOREGROUND,
    PORT_COUNT,
  };

  int port_count() const override { return PORT_COUNT; }

  char const* port_name(int i) const override {
    static std::array<char const*, PORT_COUNT> names = {
        "background", "foreground",
    };
    return names[i];
  }

  //
  // GROUP COUNT
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
    PARAM_BORDER,
    PARAM_DEBUG,
    PARAM_COUNT,
  };

  int param_count() const override { return PARAM_COUNT; }

  ParamPrototype const* param_prototype(int i) const override {
    static std::array<ParamPrototype, PARAM_COUNT> const params = {
        ParamPrototype{"border", 0, 0.2, 0, 1},
        ParamPrototype{"debug", 0, 0.0, 0, 1},
    };
    return &params[i];
  }

 public:
  int compute(Config const& config, Params const& params, Args const& args,
              cv::Mat& retimg) override;
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

template <typename Vec4T>
float manhattan_distance(Vec4T const& a, Vec4T const& b) {
  float sum = 0;
  for (int c = 0; c < 4; c++) {
    sum += std::abs(a[c] - b[c]);
  }
  return sum;
}

template <typename Vec4T>
void calculate_cost(cv::Mat& cost, cv::Mat const& dst, cv::Mat const& src,
                    int const left, int const top, int const right,
                    int const bottom) {
  cv::Size const size = cost.size();

  int y = 0;
  for (; y < top; ++y) {
    Vec4T const* b = dst.ptr<Vec4T>(y);
    Vec4T const* f = src.ptr<Vec4T>(y);
    float* cst = cost.ptr<float>(y);
    for (int x = 0; x < size.width; x++) {
      cst[x] = manhattan_distance(b[x], f[x]);
    }
  }
  for (; y < bottom; ++y) {
    Vec4T const* b = dst.ptr<Vec4T>(y);
    Vec4T const* f = src.ptr<Vec4T>(y);
    float* cst = cost.ptr<float>(y);
    for (int x = 0; x < left; x++) {
      cst[x] = manhattan_distance(b[x], f[x]);
    }
    for (int x = right; x < size.width; x++) {
      cst[x] = manhattan_distance(b[x], f[x]);
    }
  }
  for (; y < size.height; ++y) {
    Vec4T const* b = dst.ptr<Vec4T>(y);
    Vec4T const* f = src.ptr<Vec4T>(y);
    float* cst = cost.ptr<float>(y);
    for (int x = 0; x < size.width; x++) {
      cst[x] = manhattan_distance(b[x], f[x]);
    }
  }
}

void calculate_path(cv::Mat& cost, int const left, int const top,
                    int const right, int const bottom) {
  // (h) | (a) | (b)
  // ----+-----+-----
  // (g) |     | (c)
  // ----+-----+-----
  // (f) | (e) | (d)
  cv::Size const size = cost.size();

  // (a) w/o first column
  for (int x = left + 1; x < right; ++x) {
    int y = top - 1;
    cost.at<float>(y, x) +=
        std::min(cost.at<float>(y + 0, x - 1), cost.at<float>(y - 1, x - 1));
    for (--y; y > 0; --y) {
      cost.at<float>(y, x) += std::min(
          std::min(cost.at<float>(y + 1, x - 1), cost.at<float>(y + 0, x - 1)),
          cost.at<float>(y - 1, x - 1));
    }
    cost.at<float>(y, x) +=
        std::min(cost.at<float>(y + 1, x - 1), cost.at<float>(y + 0, x - 1));
  }

  // (b)
  for (int x = right; x < size.width; ++x) {
    int y = 0;
    cost.at<float>(y, x) += cost.at<float>(y + 0, x - 1);
    for (++y; y < top; ++y) {
      cost.at<float>(y, x) += std::min(
          std::min(cost.at<float>(y + 0, x - 1), cost.at<float>(y - 1, x - 1)),
          cost.at<float>(y - 1, x + 0));
    }
  }

  // (c)
  for (int y = top; y < bottom; ++y) {
    int x = size.width - 1;
    cost.at<float>(y, x) +=
        std::min(cost.at<float>(y - 1, x + 0), cost.at<float>(y - 1, x - 1));
    for (--x; x > right; --x) {
      cost.at<float>(y, x) += std::min(
          std::min(cost.at<float>(y - 1, x + 1), cost.at<float>(y - 1, x + 0)),
          cost.at<float>(y - 1, x - 1));
    }
    cost.at<float>(y, x) +=
        std::min(cost.at<float>(y - 1, x + 1), cost.at<float>(y - 1, x + 0));
  }

  // (d)
  for (int y = bottom; y < size.height; y++) {
    int x = size.width - 1;
    cost.at<float>(y, x) += cost.at<float>(y - 1, x + 0);
    for (--x; x >= right; --x) {
      cost.at<float>(y, x) += std::min(
          std::min(cost.at<float>(y + 0, x + 1), cost.at<float>(y - 1, x + 1)),
          cost.at<float>(y - 1, x + 0));
    }
  }

  // (e)
  for (int x = right - 1; x >= left; --x) {
    int y = size.height - 1;
    cost.at<float>(y, x) +=
        std::min(cost.at<float>(y + 0, x + 1), cost.at<float>(y - 1, x + 1));
    for (--y; y > bottom; --y) {
      cost.at<float>(y, x) += std::min(
          std::min(cost.at<float>(y + 1, x + 1), cost.at<float>(y + 0, x + 1)),
          cost.at<float>(y - 1, x + 1));
    }
    cost.at<float>(y, x) +=
        std::min(cost.at<float>(y + 1, x + 1), cost.at<float>(y + 0, x + 1));
  }

  // (f)
  for (int x = right - 1; x >= 0; --x) {
    int y = size.height - 1;
    cost.at<float>(y, x) += cost.at<float>(y, x + 1);
    for (--y; y >= bottom; --y) {
      cost.at<float>(y, x) += std::min(
          std::min(cost.at<float>(y + 0, x + 1), cost.at<float>(y + 1, x + 1)),
          cost.at<float>(y + 1, x + 0));
    }
  }

  // (g)
  for (int y = bottom - 1; y >= top; --y) {
    int x = left - 1;
    cost.at<float>(y, x) +=
        std::min(cost.at<float>(y + 1, x + 0), cost.at<float>(y + 1, x - 1));
    for (--x; x > 0; --x) {
      cost.at<float>(y, x) += std::min(
          std::min(cost.at<float>(y + 1, x + 1), cost.at<float>(y + 1, x + 0)),
          cost.at<float>(y + 1, x - 1));
    }
    cost.at<float>(y, x) +=
        std::min(cost.at<float>(y + 1, x + 1), cost.at<float>(y + 1, x + 0));
  }

  // (h)
  for (int y = top - 1; y >= 0; --y) {
    int x = 0;
    cost.at<float>(y, x) += cost.at<float>(y + 1, x + 0);
    for (++x; x < left; ++x) {
      cost.at<float>(y, x) += std::min(
          std::min(cost.at<float>(y + 0, x - 1), cost.at<float>(y + 1, x - 1)),
          cost.at<float>(y + 1, x + 0));
    }
  }

  // (a) first column
  {
    int x = left;
    int y = top - 1;
    cost.at<float>(y, x) +=
        std::min(cost.at<float>(y + 0, x - 1), cost.at<float>(y - 1, x - 1));
    for (--y; y > 0; --y) {
      cost.at<float>(y, x) += std::min(
          std::min(cost.at<float>(y + 1, x - 1), cost.at<float>(y + 0, x - 1)),
          cost.at<float>(y - 1, x - 1));
    }
    cost.at<float>(y, x) +=
        std::min(cost.at<float>(y + 1, x - 1), cost.at<float>(y + 0, x - 1));
  }
}

// find a position which has minimum cost
cv::Point find_next_pos(std::array<cv::Point, 3> const& ps, cv::Mat const& cost,
                        int const left, int const top, int const right,
                        int const bottom) {
  cv::Size const size = cost.size();

  float min_cost = std::numeric_limits<float>::infinity();
  cv::Point min_pos;
  for (cv::Point const& cur : ps) {
    if ((cur.x < 0) || (cur.x >= size.width) || (cur.y < 0) ||
        (cur.y >= size.height)) {
      continue;
    }
    if ((left <= cur.x) && (cur.x < right) && (top <= cur.y) &&
        (cur.y < bottom)) {
      continue;
    }

    float const cc = cost.at<float>(cur);
    if (min_cost > cc) {
      min_cost = cc, min_pos = cur;
    }
  }
  return min_pos;
}

template <typename Vec4T>
void copy_vertically(cv::Mat& dst, cv::Mat const& src, int const x,
                     int const top, int const bottom) {
  for (int y = top; y < bottom; ++y) {
    dst.at<Vec4T>(y, x) = src.at<Vec4T>(y, x);
  }
}

template <typename Vec4T>
void copy_horizontally(cv::Mat& dst, cv::Mat const& src, int const left,
                       int const right, int const y) {
  for (int x = left; x < right; ++x) {
    dst.at<Vec4T>(y, x) = src.at<Vec4T>(y, x);
  }
}

template <typename Vec4T>
void trace_path(cv::Mat& dst, cv::Mat const& src, cv::Mat const& cost,
                int const left, int const top, int const right,
                int const bottom, bool const debug) {
  // (h) | (a) | (b)
  // ----+-----+-----
  // (g) |     | (c)
  // ----+-----+-----
  // (f) | (e) | (d)
  using value_type = typename Vec4T::value_type;
  int const g = std::numeric_limits<value_type>::max();
  Vec4T const mark_color(g, 0, g, g);

  cv::Size const size = cost.size();

  // (a)
  cv::Point pos(left, 0);
  {
    // find a starting point
    float min_cost = std::numeric_limits<float>::infinity();
    for (int yy = 0; yy < top; ++yy) {
      float const cc = cost.at<float>(yy, pos.x);  // current cost
      if (min_cost > cc) {
        min_cost = cc, pos.y = yy;
      }
    }

    // copy
    copy_vertically<Vec4T>(dst, src, pos.x, pos.y, top);
    if (debug) {
      dst.at<Vec4T>(pos) = mark_color;
    }

    // (a) to (h)
    std::array<cv::Point, 3> const ps = {
        cv::Point(pos.x - 1, pos.y - 1), cv::Point(pos.x - 1, pos.y + 0),
        cv::Point(pos.x - 1, pos.y + 1),
    };
    pos = find_next_pos(ps, cost, left, top, right, bottom);
  }

  // (h)
  copy_vertically<Vec4T>(dst, src, pos.x, pos.y, top);
  if (debug) {
    dst.at<Vec4T>(pos) = mark_color;
  }
  while ((pos.x != 0) && (pos.y != top)) {
    std::array<cv::Point, 3> const ps = {
        cv::Point(pos.x - 1, pos.y + 0), cv::Point(pos.x - 1, pos.y + 1),
        cv::Point(pos.x + 0, pos.y + 1),
    };

    cv::Point const cur = find_next_pos(ps, cost, left, top, right, bottom);
    if (pos.x != cur.x) {
      copy_vertically<Vec4T>(dst, src, cur.x, cur.y, top);
      if (debug) {
        dst.at<Vec4T>(cur) = mark_color;
      }
    }
    pos = cur;  // move
  }
  pos.y = top;

  // (g)
  while (pos.y < bottom) {
    copy_horizontally<Vec4T>(dst, src, pos.x, left, pos.y);
    if (debug) {
      dst.at<Vec4T>(pos) = mark_color;
    }

    std::array<cv::Point, 3> const ps = {
        cv::Point(pos.x - 1, pos.y + 1), cv::Point(pos.x + 0, pos.y + 1),
        cv::Point(pos.x + 1, pos.y + 1),
    };

    pos = find_next_pos(ps, cost, left, top, right, bottom);
  }

  // (f)
  copy_horizontally<Vec4T>(dst, src, pos.x, left, pos.y);
  if (debug) {
    dst.at<Vec4T>(pos) = mark_color;
  }
  while ((pos.x != left) && (pos.y != size.height - 1)) {
    std::array<cv::Point, 3> const ps = {
        cv::Point(pos.x + 0, pos.y + 1), cv::Point(pos.x + 1, pos.y + 1),
        cv::Point(pos.x + 1, pos.y + 0),
    };

    cv::Point const cur = find_next_pos(ps, cost, left, top, right, bottom);
    if (pos.y != cur.y) {
      copy_horizontally<Vec4T>(dst, src, cur.x, left, cur.y);
      if (debug) {
        dst.at<Vec4T>(cur) = mark_color;
      }
    }
    pos = cur;  // move
  }
  pos.x = left;

  // (e)
  while (pos.x < right) {
    copy_vertically<Vec4T>(dst, src, pos.x, bottom, pos.y + 1);
    if (debug) {
      dst.at<Vec4T>(pos) = mark_color;
    }

    std::array<cv::Point, 3> const ps = {
        cv::Point(pos.x + 1, pos.y + 1), cv::Point(pos.x + 1, pos.y + 0),
        cv::Point(pos.x + 1, pos.y - 1),
    };

    pos = find_next_pos(ps, cost, left, top, right, bottom);
  }

  // (d)
  copy_vertically<Vec4T>(dst, src, pos.x, bottom, pos.y + 1);
  if (debug) {
    dst.at<Vec4T>(pos) = mark_color;
  }
  while ((pos.x != size.width - 1) && (pos.y != bottom - 1)) {
    std::array<cv::Point, 3> const ps = {
        cv::Point(pos.x + 1, pos.y + 0), cv::Point(pos.x + 1, pos.y - 1),
        cv::Point(pos.x + 0, pos.y - 1),
    };

    cv::Point const cur = find_next_pos(ps, cost, left, top, right, bottom);
    if (pos.x < cur.x) {
      copy_vertically<Vec4T>(dst, src, cur.x, bottom, cur.y + 1);
      if (debug) {
        dst.at<Vec4T>(cur) = mark_color;
      }
    }
    pos = cur;  // move
  }
  pos.y = bottom - 1;

  // (c)
  while (pos.y > top) {
    copy_horizontally<Vec4T>(dst, src, right, pos.x + 1, pos.y);
    if (debug) {
      dst.at<Vec4T>(pos) = mark_color;
    }

    std::array<cv::Point, 3> const ps = {
        cv::Point(pos.x + 1, pos.y - 1), cv::Point(pos.x + 0, pos.y - 1),
        cv::Point(pos.x - 1, pos.y - 1),
    };

    pos = find_next_pos(ps, cost, left, top, right, bottom);
  }

  // (b)
  copy_horizontally<Vec4T>(dst, src, right, pos.x + 1, pos.y);
  if (debug) {
    dst.at<Vec4T>(pos) = mark_color;
  }
  while ((pos.x != right - 1) && (pos.y != 0)) {
    std::array<cv::Point, 3> const ps = {
        cv::Point(pos.x + 0, pos.y - 1), cv::Point(pos.x - 1, pos.y - 1),
        cv::Point(pos.x - 1, pos.y + 0),
    };

    cv::Point const cur = find_next_pos(ps, cost, left, top, right, bottom);
    if (pos.y > cur.y) {
      copy_horizontally<Vec4T>(dst, src, right, cur.x + 1, cur.y);
      if (debug) {
        dst.at<Vec4T>(cur) = mark_color;
      }
    }
    pos = cur;  // move
  }
  pos.x = right - 1;

  // (a)
  while (pos.x > left) {
    copy_vertically<Vec4T>(dst, src, pos.x, pos.y, top);
    if (debug) {
      dst.at<Vec4T>(pos) = mark_color;
    }

    std::array<cv::Point, 3> const ps = {
        cv::Point(pos.x - 1, pos.y - 1), cv::Point(pos.x - 1, pos.y + 0),
        cv::Point(pos.x - 1, pos.y + 1),
    };

    pos = find_next_pos(ps, cost, left, top, right, bottom);
  }
}

int MyFx::compute(Config const& config, Params const& params, Args const& args,
                  cv::Mat& retimg) try {
  DEBUG_PRINT(__FUNCTION__);

  if (args.invalid(PORT_BACKGROUND) && args.invalid(PORT_FOREGROUND)) {
    return 0;
  }

  if (args.invalid(PORT_BACKGROUND)) {
    args.get(PORT_BACKGROUND).copyTo(retimg(args.rect(PORT_BACKGROUND)));
    return 0;
  }

  if (args.invalid(PORT_FOREGROUND)) {
    args.get(PORT_FOREGROUND).copyTo(retimg(args.rect(PORT_FOREGROUND)));
    return 0;
  }

  cv::Mat const src = args.get(PORT_FOREGROUND);
  cv::Size const size = src.size();  // foreground size
  if ((size.width < 3) || (size.height < 3)) {
    return 0;
  }

  int const border = params.get<int>(PARAM_BORDER, size.height / 2);
  if (border <= 0) {
    args.get(PORT_FOREGROUND).copyTo(retimg(args.rect(PORT_FOREGROUND)));
    return 0;
  }

  bool const debug = params.get<bool>(PARAM_DEBUG);

  cv::Rect roi;
  roi.x = border;
  roi.y = border;
  roi.width = size.width - 2 * border;
  roi.height = size.height - 2 * border;
  if ((roi.width <= 0) || (roi.height <= 0)) {
    return 0;
  }

  // background
  args.get(PORT_BACKGROUND).copyTo(retimg(args.rect(PORT_BACKGROUND)));

  // foreground
  cv::Mat dst = retimg(cv::Rect(args.offset(PORT_FOREGROUND), size));
  args.get(PORT_FOREGROUND)(roi).copyTo(dst(roi));

  // quilting cost
  cv::Mat cost(size, CV_32FC1, std::numeric_limits<cv::Scalar>::infinity());

  int const left = roi.x;
  int const right = roi.x + roi.width;
  int const top = roi.y;
  int const bottom = roi.y + roi.height;

  if (retimg.type() == CV_8UC4) {
    // calculate cost
    calculate_cost<cv::Vec4b>(cost, dst, src, left, top, right, bottom);
  } else {
    // calculate cost
    calculate_cost<cv::Vec4w>(cost, dst, src, left, top, right, bottom);
  }

  // calculate a path by dynamic programming
  calculate_path(cost, left, top, right, bottom);

  // trace the path
  if (retimg.type() == CV_8UC4) {
    trace_path<cv::Vec4b>(dst, src, cost, left, top, right, bottom, debug);
  } else {
    trace_path<cv::Vec4w>(dst, src, cost, left, top, right, bottom, debug);
  }

  if (debug) {
    double const g = (retimg.type() == CV_8UC4)
                         ? std::numeric_limits<uchar>::max()
                         : std::numeric_limits<ushort>::max();
    cv::rectangle(dst, roi, cv::Scalar(g, g, 0, g));
  }

  return 0;
} catch (cv::Exception const& e) {
  DEBUG_PRINT(e.what());
}
