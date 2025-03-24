#ifndef FOCUS_PEAKING__FOCUS_PEAKING_HPP_
#define FOCUS_PEAKING__FOCUS_PEAKING_HPP_

#include <image_transport/image_transport.hpp>
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
  image_transport::ImageTransport it_;
  image_transport::Subscriber image_sub_;
};
}  // namespace focus_peaking

#endif  // FOCUS_PEAKING__FOCUS_PEAKING_HPP_