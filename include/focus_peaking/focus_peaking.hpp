#ifndef FOCUS_PEAKING__FOCUS_PEAKING_HPP_
#define FOCUS_PEAKING__FOCUS_PEAKING_HPP_

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>

namespace focus_peaking
{
class FocusPeaking : public rclcpp::Node
{
public:
  explicit FocusPeaking(const rclcpp::NodeOptions & options);

  void image_callback(const sensor_msgs::msg::Image::ConstSharedPtr & msg);

private:
  std::shared_ptr<rclcpp::Subscription<sensor_msgs::msg::Image>> image_sub_;

  std::string viz_window_name_;
  int64_t denoising_kernel_size_;
  int64_t edge_dilation_kernel_size_;
};
}  // namespace focus_peaking

#endif  // FOCUS_PEAKING__FOCUS_PEAKING_HPP_