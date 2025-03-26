#include "focus_peaking/focus_peaking.hpp"

#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

namespace focus_peaking
{
FocusPeaking::FocusPeaking(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
: Node("focus_peaking", options), viz_window_name_("FocusPeaking")
{
  image_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "/image_raw", 5,
    [&](const sensor_msgs::msg::Image::ConstSharedPtr & msg) { image_callback(msg); });

  cv::namedWindow(viz_window_name_, cv::WINDOW_NORMAL);

  RCLCPP_INFO(get_logger(), "FocusPeaking initialized");
}

void FocusPeaking::image_callback(const sensor_msgs::msg::Image::ConstSharedPtr & msg)
{
  RCLCPP_DEBUG(get_logger(), "Image received, processing...");

  cv::Mat image = cv_bridge::toCvShare(msg, sensor_msgs::image_encodings::BGR8)->image;

  // Convert to grayscale and detect edges
  cv::Mat gray, edges;
  cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
  cv::GaussianBlur(gray, gray, cv::Size(3, 3), 0);
  cv::Canny(gray, edges, 50, 150);

  // dilate edges for better visibility
  cv::Mat kernel = cv::getStructuringElement(cv::MORPH_DILATE, cv::Size(3, 3));
  cv::dilate(edges, edges, kernel);

  // Create a red mask where edges are detected
  cv::Mat red_edges = cv::Mat::zeros(image.size(), image.type());
  red_edges.setTo(cv::Scalar(0, 0, 255), edges);  // Only red channel is updated

  // Blend red edges with the original image
  cv::Mat result;
  cv::addWeighted(image, 1.0, red_edges, 1.0, 0, result);

  cv::imshow(viz_window_name_, result);
  cv::waitKey(1);
}
}  // namespace focus_peaking

// Register the node as a component
#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(focus_peaking::FocusPeaking)