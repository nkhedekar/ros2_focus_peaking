#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>

class ROISelector
{
private:
  cv::Point start_point, end_point;
  bool drawing = false;
  bool roi_complete = false;
  cv::Rect current_roi;
  std::string window_name;

public:
  ROISelector(const std::string & win_name) : window_name(win_name) {}

  void set_mouse_callback() { cv::setMouseCallback(window_name, mouseCallback, this); }

  static void mouseCallback(int event, int x, int y, int flags, void * userdata)
  {
    ROISelector * selector = static_cast<ROISelector *>(userdata);
    selector->handleMouse(event, x, y, flags);
  }

  void handleMouse(int event, int x, int y, int /*flags*/)
  {
    switch (event) {
      case cv::EVENT_LBUTTONDOWN:
        drawing = true;
        roi_complete = false;
        start_point = cv::Point(x, y);
        end_point = cv::Point(x, y);
        break;

      case cv::EVENT_MOUSEMOVE:
        if (drawing) {
          end_point = cv::Point(x, y);
        }
        break;

      case cv::EVENT_LBUTTONUP:
        if (drawing) {
          drawing = false;
          end_point = cv::Point(x, y);

          // Create ROI rectangle
          current_roi = cv::Rect(
            std::min(start_point.x, end_point.x), std::min(start_point.y, end_point.y),
            std::abs(end_point.x - start_point.x), std::abs(end_point.y - start_point.y));

          if (current_roi.width > 5 && current_roi.height > 5) {
            roi_complete = true;
          }
        }
        break;
    }
  }

  void drawROI(cv::Mat & image)
  {
    if (drawing) {
      // Draw current selection
      cv::rectangle(image, start_point, end_point, cv::Scalar(0, 255, 0), 2);
    } else if (roi_complete) {
      // Draw completed ROI
      cv::rectangle(image, current_roi, cv::Scalar(0, 0, 255), 2);
    }
  }

  bool isROIComplete() { return roi_complete; }

  cv::Rect getROI() { return current_roi; }

  void reset()
  {
    drawing = false;
    roi_complete = false;
  }
};
