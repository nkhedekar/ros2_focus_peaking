#ifndef FOCUS_PEAKING__ROI_SELECTOR_HPP_
#define FOCUS_PEAKING__ROI_SELECTOR_HPP_

#include <functional>
#include <opencv2/opencv.hpp>
#include <optional>
#include <string>

namespace focus_peaking
{
class ROISelector
{
private:
  cv::Point start_point_, end_point_;
  bool drawing_ = false;
  bool roi_complete_ = false;
  int min_roi_size_ = 5;
  cv::Rect current_roi_;
  std::string window_name_;
  std::function<void(const cv::Rect &)> roi_callback_;

public:
  explicit ROISelector(const std::string & win_name);

  void set_mouse_callback();

  static void mouse_callback(int event, int x, int y, int flags, void * userdata);

  void register_roi_callback(std::function<void(const cv::Rect &)> && callback);

  void handle_mouse(int event, int x, int y, int flags);

  void draw(cv::Mat & image);

  bool is_roi_complete() const;

  std::optional<cv::Rect> get_roi() const;

  void reset();

  void clear_roi_callback();
};
}  // namespace focus_peaking

#endif  // FOCUS_PEAKING__ROI_SELECTOR_HPP_