#ifndef FOCUS_PEAKING__FOCUS_WIDGET_HPP_
#define FOCUS_PEAKING__FOCUS_WIDGET_HPP_

#include <deque>
#include <opencv2/core/mat.hpp>

namespace focus_peaking
{
class FocusWidget
{
public:
  FocusWidget(double width_ratio, double height_ratio, int margin_px, size_t history_size);
  ~FocusWidget() = default;

  void update(double score);
  double get_min_score();
  double get_max_score();
  void draw(cv::Mat & display_image);

private:
  double width_ratio_;
  double height_ratio_;
  int margin_px_;
  size_t history_size_;
  std::deque<double> scores_history_;
  double min_score_;
  double max_score_;
};
}  // namespace focus_peaking

#endif  // FOCUS_PEAKING__FOCUS_WIDGET_HPP_