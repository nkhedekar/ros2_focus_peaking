#include "focus_peaking/focus_peaking.hpp"

#include <cv_bridge/cv_bridge.hpp>

namespace focus_peaking
{
FocusPeaking::FocusPeaking(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
: Node("focus_peaking", options), it_(shared_from_this())
{
  image_sub_ = it_.subscribe("/camera/image_raw", 1, &FocusPeaking::image_callback, this);

  RCLCPP_INFO(get_logger(), "FocusPeaking initialized");
}

void FocusPeaking::image_callback(const sensor_msgs::msg::Image::ConstSharedPtr & msg)
{
  RCLCPP_DEBUG(get_logger(), "Image received, processing...");

  try {
    cv::Mat image = cv_bridge::toCvShare(msg, sensor_msgs::image_encodings::BGR8)->image;

    // Process the image here
  } catch (cv_bridge::Exception & e) {
    RCLCPP_ERROR(get_logger(), "cv_bridge exception: %s", e.what());
  }
}
}  // namespace focus_peaking

// Register the node as a component
#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(focus_peaking::FocusPeaking)