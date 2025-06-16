#include "focus_peaking/focus_widget.hpp"

#include <opencv2/imgproc.hpp>

namespace focus_peaking
{

FocusWidget::FocusWidget(double width_ratio, size_t history_size)
: width_ratio_(width_ratio), history_size_(history_size)
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

  // Widget background dimensions: width_px wide, full image height
  int widget_bg_width = static_cast<int>(width_ratio_ * img_w);
  int widget_bg_height = img_h;

  // Ensure minimum width for visibility
  widget_bg_width = std::max(50, widget_bg_width);

  // Position the white background on the right side of the image
  cv::Point bg_tl(img_w - widget_bg_width, 0);
  cv::Point bg_br(img_w, img_h);
  cv::Rect bg_rect(bg_tl, bg_br);

  // Draw white background
  cv::rectangle(display_image, bg_rect, cv::Scalar(255, 255, 255), cv::FILLED);
  cv::rectangle(display_image, bg_rect, cv::Scalar(0, 0, 0), 3);

  // Calculate the inner widget rectangle (centered in the white background)
  int inner_margin = 0.25 * widget_bg_width;  // Margin around the inner widget
  int inner_width = widget_bg_width - 2 * inner_margin;
  int inner_height = img_h - 6 * inner_margin;

  // Ensure minimum dimensions for the inner widget
  inner_width = std::max(20, inner_width);
  inner_height = std::max(40, inner_height);

  // Center the inner widget in the white background
  int inner_x = bg_tl.x + (widget_bg_width - inner_width) / 2;
  int inner_y = bg_tl.y + (widget_bg_height - inner_height) / 2;

  cv::Point inner_tl(inner_x, inner_y);
  cv::Point inner_br(inner_x + inner_width, inner_y + inner_height);
  cv::Rect inner_rect(inner_tl, inner_br);

  // Draw circle indicators OUTSIDE the inner rectangle
  int circle_radius = std::min(15, widget_bg_width / 8);

  // Sharp circle above the inner rectangle
  cv::Point sharp_center(bg_tl.x + widget_bg_width / 2, inner_y - inner_margin - circle_radius);
  cv::circle(display_image, sharp_center, circle_radius, cv::Scalar(0, 255, 0), cv::FILLED);

  // Blur circle below the inner rectangle
  cv::Point blur_center(
    bg_tl.x + widget_bg_width / 2, inner_y + inner_height + inner_margin + circle_radius);
  cv::circle(display_image, blur_center, circle_radius, cv::Scalar(0, 0, 255), cv::FILLED);

  // Draw the red focus indicator line inside the inner rectangle
  if (!scores_history_.empty()) {
    double current_score = scores_history_.back();

    if (max_score_ > min_score_ && (max_score_ - min_score_) > 1e-6) {
      // Normalize score to [0, 1]
      double normalized_score = (current_score - min_score_) / (max_score_ - min_score_);
      normalized_score = std::max(0.0, std::min(1.0, normalized_score));

      // Y-coordinate for the line: top of inner rectangle for score=1.0, bottom for score=0.0
      int line_y = inner_tl.y + static_cast<int>((1.0 - normalized_score) * inner_height);

      // Draw rectangle from level to bottom
      cv::rectangle(
        display_image, cv::Point(inner_tl.x, line_y), cv::Point(inner_br.x, inner_br.y),
        cv::Scalar(172, 92, 14), cv::FILLED);

      // if score below half of normalized score then draw a red line, green if above
      if (normalized_score < 0.5) {
        cv::line(
          display_image, cv::Point(inner_tl.x, line_y), cv::Point(inner_br.x, line_y),
          cv::Scalar(0, 0, 255), 3);
      } else {
        cv::line(
          display_image, cv::Point(inner_tl.x, line_y), cv::Point(inner_br.x, line_y),
          cv::Scalar(0, 255, 0), 3);
      }
    } else {
      // If min_score_ is very close to max_score_, draw gray line in the middle
      int line_y = inner_tl.y + inner_height / 2;
      cv::line(
        display_image, cv::Point(inner_tl.x + 5, line_y), cv::Point(inner_br.x - 5, line_y),
        cv::Scalar(128, 128, 128),
        2);  // Gray line
    }
  }

  // Draw the inner widget rectangle with black border
  cv::rectangle(display_image, inner_rect, cv::Scalar(0, 0, 0), 3);
}

void FocusWidget::reset() { scores_history_.clear(); }

}  // namespace focus_peaking