#pragma once
#include <functional>
#include <opencv2/opencv.hpp>
#include <optional>
#include <string>
#include <thread>

class ROISelector
{
private:
  cv::Point start_point_, end_point_;
  bool drawing_ = false;
  bool roi_complete_ = false;
  cv::Rect current_roi_;
  std::string window_name_;
  std::function<void(const cv::Rect &)> roi_callback_;

public:
  ROISelector(const std::string & win_name) : window_name_(win_name) {}

  void set_mouse_callback() { cv::setMouseCallback(window_name_, mouse_callback, this); }

  static void mouse_callback(int event, int x, int y, int flags, void * userdata)
  {
    ROISelector * selector = static_cast<ROISelector *>(userdata);
    selector->handle_mouse(event, x, y, flags);
  }

  void register_roi_callback(std::function<void(const cv::Rect &)> callback)
  {
    roi_callback_ = std::move(callback);
  }

  void handle_mouse(int event, int x, int y, int /*flags*/)
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

          // Create ROI rectangle
          current_roi_ = cv::Rect(
            std::min(start_point_.x, end_point_.x), std::min(start_point_.y, end_point_.y),
            std::abs(end_point_.x - start_point_.x), std::abs(end_point_.y - start_point_.y));

          if (current_roi_.width > 5 && current_roi_.height > 5) {
            roi_complete_ = true;
            roi_callback_(current_roi_);
          }
        }
        break;
    }
  }

  void draw(cv::Mat & image)
  {
    if (drawing_) {
      cv::rectangle(image, start_point_, end_point_, cv::Scalar(255, 0, 0), 2);
    } else if (!current_roi_.empty()) {
      cv::rectangle(image, current_roi_, cv::Scalar(0, 255, 0), 2);
    }
  }

  bool is_roi_complete() { return roi_complete_; }

  std::optional<cv::Rect> get_roi()
  {
    if (roi_complete_) {
      return current_roi_;
    } else {
      return std::nullopt;
    }
  }

  void reset()
  {
    drawing_ = false;
    roi_complete_ = false;
    current_roi_ = cv::Rect();
  }
};