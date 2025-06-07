#include "focus_peaking/focus_widget.hpp"

#include <opencv2/imgproc.hpp>

namespace focus_peaking
{
FocusWidget::FocusWidget(
  double width_ratio, double height_ratio, int margin_px, size_t history_size)
: width_ratio_(width_ratio),
  height_ratio_(height_ratio),
  margin_px_(margin_px),
  history_size_(history_size)
{
}

void FocusWidget::update(double score)
{
  scores_history_.push_back(score);
  if (scores_history_.size() > history_size_) {
    scores_history_.pop_front();
  }
  min_score_ = *std::min_element(scores_history_.begin(), scores_history_.end());
  max_score_ = *std::max_element(scores_history_.begin(), scores_history_.end());
}

double FocusWidget::get_min_score() { return min_score_; }
double FocusWidget::get_max_score() { return max_score_; }

void FocusWidget::draw(cv::Mat & display_image)
{
  int img_h = display_image.rows;
  int img_w = display_image.cols;

  // Calculate actual widget dimensions based on ratios
  int actual_w_px = static_cast<int>(img_w * width_ratio_);
  int actual_h_px = static_cast<int>(img_h * height_ratio_);

  // Ensure minimum dimensions for visibility
  actual_w_px = std::max(10, actual_w_px);
  actual_h_px = std::max(20, actual_h_px);

  // Top-right corner for the widgets background
  cv::Point tl(img_w - actual_w_px - margin_px_, margin_px_);
  cv::Point br(img_w - margin_px_, margin_px_ + actual_h_px);
  cv::Rect bg_rect(tl, br);

  // white background
  cv::rectangle(display_image, bg_rect, cv::Scalar(255, 255, 255), cv::FILLED);
  // black border
  cv::rectangle(display_image, bg_rect, cv::Scalar(0, 0, 0), 2);

  double current_score = scores_history_.back();

  // Draw the red focus indicator line
  if (
    max_score_ > min_score_ &&
    (max_score_ - min_score_) > 1e-6) {  // Avoid division by zero or tiny differences
    double normalized_score = (current_score - min_score_) / (max_score_ - min_score_);
    normalized_score = std::max(0.0, std::min(1.0, normalized_score));  // Clamp to [0, 1]

    // Y-coordinate for the line: top of widget for score=1.0, bottom for score=0.0
    int line_y = tl.y + static_cast<int>((1.0 - normalized_score) * actual_h_px);

    cv::line(
      display_image, cv::Point(tl.x, line_y), cv::Point(br.x, line_y), cv::Scalar(0, 0, 255),
      3);  // Red line
  } else {
    // If min_score_ is very close to max_score_ (e.g., first few frames, static scene, or history_size=1)
    // Draw line in the middle with a different color (e.g., gray) to indicate neutral/undetermined
    int line_y = tl.y + actual_h_px / 2;
    cv::line(
      display_image, cv::Point(tl.x, line_y), cv::Point(br.x, line_y), cv::Scalar(128, 128, 128),
      2);  // Gray line
  }
}
}  // namespace focus_peaking
