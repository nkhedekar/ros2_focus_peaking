#include "focus_peaking/roi_selector.hpp"

namespace focus_peaking
{

ROISelector::ROISelector(const std::string & win_name, int min_roi_size)
: drawing_(false), roi_complete_(false), min_roi_size_(min_roi_size), window_name_(win_name)
{
}

void ROISelector::set_mouse_callback() { cv::setMouseCallback(window_name_, mouse_callback, this); }

void ROISelector::mouse_callback(int event, int x, int y, int flags, void * userdata)
{
  ROISelector * selector = static_cast<ROISelector *>(userdata);
  selector->handle_mouse(event, x, y, flags);
}

void ROISelector::register_roi_callback(std::function<void(const cv::Rect &)> && callback)
{
  roi_callback_ = std::move(callback);
}

void ROISelector::handle_mouse(int event, int x, int y, int /*flags*/)
{
  switch (event) {
    case cv::EVENT_LBUTTONDOWN:
      drawing_ = true;
      roi_complete_ = false;
      start_point_ = cv::Point(x, y);
      end_point_ = cv::Point(x, y);
      break;

    case cv::EVENT_MOUSEMOVE:
      if (drawing_) {
        end_point_ = cv::Point(x, y);
      }
      break;

    case cv::EVENT_LBUTTONUP:
      if (drawing_) {
        drawing_ = false;
        end_point_ = cv::Point(x, y);

        current_roi_ = cv::Rect(
          std::min(start_point_.x, end_point_.x), std::min(start_point_.y, end_point_.y),
          std::abs(end_point_.x - start_point_.x), std::abs(end_point_.y - start_point_.y));

        if (current_roi_.width > min_roi_size_ && current_roi_.height > min_roi_size_) {
          roi_complete_ = true;
          if (roi_callback_) {
            roi_callback_(current_roi_);
          }
        }
      }
      break;
  }
}

void ROISelector::draw(cv::Mat & image)
{
  if (drawing_) {
    cv::rectangle(image, start_point_, end_point_, cv::Scalar(255, 0, 0), 2);
  } else if (!current_roi_.empty()) {
    cv::rectangle(image, current_roi_, cv::Scalar(0, 255, 0), 2);
  }
}

bool ROISelector::is_roi_complete() const { return roi_complete_; }

std::optional<cv::Rect> ROISelector::get_roi() const
{
  if (!current_roi_.empty()) {
    return current_roi_;
  } else {
    return std::nullopt;
  }
}

void ROISelector::reset()
{
  drawing_ = false;
  roi_complete_ = false;
  current_roi_ = cv::Rect();
}

void ROISelector::clear_roi_callback() { roi_callback_ = nullptr; }
}  // namespace focus_peaking