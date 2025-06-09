#ifndef FOCUS_PEAKING__FOCUS_PEAKING_HPP_
#define FOCUS_PEAKING__FOCUS_PEAKING_HPP_

#include <memory>
#include <opencv2/core/mat.hpp>
#include <optional>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>

#include "focus_peaking/focus_widget.hpp"
#include "focus_peaking/roi_selector.hpp"

namespace focus_peaking
{
class FocusPeaking : public rclcpp::Node
{
public:
  explicit FocusPeaking(const rclcpp::NodeOptions & options);

  void image_callback(const sensor_msgs::msg::Image::ConstSharedPtr & msg);

  double calculate_focus_score(const cv::Mat & gray_image);

private:
  std::shared_ptr<rclcpp::Subscription<sensor_msgs::msg::Image>> image_sub_;

  std::string viz_window_name_;
  int64_t denoising_kernel_size_;
  int64_t edge_dilation_kernel_size_;
  bool widget_enabled_;
  double widget_width_ratio_;
  size_t focus_history_size_;
  std::unique_ptr<FocusWidget> focus_widget_;
  ROISelector roi_selector_;
  std::optional<cv::Rect> roi_;
};
}  // namespace focus_peaking

#endif  // FOCUS_PEAKING__FOCUS_PEAKING_HPP_