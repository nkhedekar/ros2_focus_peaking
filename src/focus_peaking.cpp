#include "focus_peaking/focus_peaking.hpp"

#include <algorithm>
#include <cv_bridge/cv_bridge.hpp>
#include <deque>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

namespace focus_peaking
{

rcl_interfaces::msg::ParameterDescriptor create_param_descriptor(
  const std::string & description, const rclcpp::ParameterType & param_type)
{
  rcl_interfaces::msg::ParameterDescriptor descriptor;
  descriptor.description = description;
  descriptor.type = static_cast<uint8_t>(param_type);
  return descriptor;
}

FocusPeaking::FocusPeaking(const rclcpp::NodeOptions & options)
: Node("focus_peaking", options),
  viz_window_name_("focus_peaking"),
  denoising_kernel_size_(3),
  edge_dilation_kernel_size_(3),
  widget_enabled_(true),
  widget_width_ratio_(0.04),
  focus_history_size_(200),
  roi_selector_("focus_peaking"),
  roi_(std::nullopt)
{
  RCLCPP_INFO(get_logger(), "Initializing FocusPeaking node...");

  declare_parameter(
    "denoising_kernel_size", denoising_kernel_size_,
    create_param_descriptor(
      "Size of gaussian blur kernel used for denoising. Must be odd and positive.",
      rclcpp::ParameterType::PARAMETER_INTEGER));

  declare_parameter(
    "edge_dilation_kernel_size", edge_dilation_kernel_size_,
    create_param_descriptor(
      "Size of kernel used to dilate detected edges for peaking. Must be positive.",
      rclcpp::ParameterType::PARAMETER_INTEGER));

  declare_parameter(
    "widget_enabled", widget_enabled_,
    create_param_descriptor("Enable focus quality widget", rclcpp::ParameterType::PARAMETER_BOOL));

  declare_parameter(
    "widget_width_ratio", widget_width_ratio_,
    create_param_descriptor(
      "Widget width as a ratio of image width (0.0 to 1.0)",
      rclcpp::ParameterType::PARAMETER_DOUBLE));

  declare_parameter(
    "focus_history_size", static_cast<int64_t>(focus_history_size_),
    create_param_descriptor(
      "Number of frames for focus score history (min 1)",
      rclcpp::ParameterType::PARAMETER_INTEGER));

  denoising_kernel_size_ = get_parameter("denoising_kernel_size").as_int();
  edge_dilation_kernel_size_ = get_parameter("edge_dilation_kernel_size").as_int();
  widget_enabled_ = get_parameter("widget_enabled").as_bool();
  widget_width_ratio_ = get_parameter("widget_width_ratio").as_double();
  focus_history_size_ = get_parameter("focus_history_size").as_int();

  RCLCPP_INFO(get_logger(), "denoising_kernel_size: %ld", denoising_kernel_size_);
  RCLCPP_INFO(get_logger(), "edge_dilation_kernel_size: %ld", edge_dilation_kernel_size_);
  RCLCPP_INFO(get_logger(), "widget_enabled: %s", widget_enabled_ ? "true" : "false");
  RCLCPP_INFO(get_logger(), "widget_width_ratio: %lf", widget_width_ratio_);
  RCLCPP_INFO(get_logger(), "focus_history_size: %ld", focus_history_size_);

  focus_widget_ = std::make_unique<FocusWidget>(widget_width_ratio_, focus_history_size_);

  image_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "/image_raw", 5,
    [&](const sensor_msgs::msg::Image::ConstSharedPtr & msg) { image_callback(msg); });

  cv::namedWindow(viz_window_name_, cv::WINDOW_NORMAL);
  cv::resizeWindow(viz_window_name_, 800, 600);
  roi_selector_.register_mouse_callback();

  roi_selector_.register_roi_callback([&](const cv::Rect & roi) {
    roi_ = roi;
    focus_widget_->reset();
    RCLCPP_INFO(get_logger(), "New ROI selected");
  });

  RCLCPP_INFO(get_logger(), "FocusPeaking initialized");
}

double FocusPeaking::calculate_focus_score(const cv::Mat & gray_image)
{
  cv::Mat laplacian_img;
  cv::Laplacian(gray_image, laplacian_img, CV_64F);

  cv::Scalar mean, stddev;
  cv::meanStdDev(laplacian_img, mean, stddev);

  // Variance is the square of the standard deviation
  return stddev.val[0] * stddev.val[0];
}

void FocusPeaking::image_callback(const sensor_msgs::msg::Image::ConstSharedPtr & msg)
{
  RCLCPP_DEBUG(get_logger(), "Image received, processing...");

  cv_bridge::CvImageConstPtr cv_ptr;
  try {
    cv_ptr = cv_bridge::toCvShare(msg, sensor_msgs::image_encodings::BGR8);
  } catch (cv_bridge::Exception & e) {
    RCLCPP_ERROR(get_logger(), "cv_bridge exception: %s", e.what());
    return;
  }
  cv::Mat image = cv_ptr->image;
  cv::Mat result_display_img = image.clone();

  // --- Focus Peaking Visualization (Canny edges) ---
  cv::Mat gray_for_peaking, edges_for_peaking;
  cv::cvtColor(image, gray_for_peaking, cv::COLOR_BGR2GRAY);
  cv::GaussianBlur(
    gray_for_peaking, gray_for_peaking, cv::Size(denoising_kernel_size_, denoising_kernel_size_),
    0);
  cv::Mat focus_widget_input = gray_for_peaking.clone();
  cv::Canny(gray_for_peaking, edges_for_peaking, 50, 150);

  cv::Mat dilation_kernel = cv::getStructuringElement(
    cv::MORPH_DILATE, cv::Size(edge_dilation_kernel_size_, edge_dilation_kernel_size_));
  cv::dilate(edges_for_peaking, edges_for_peaking, dilation_kernel);

  cv::Mat red_highlight = cv::Mat::zeros(image.size(), image.type());
  red_highlight.setTo(cv::Scalar(0, 0, 255), edges_for_peaking);
  cv::addWeighted(result_display_img, 1.0, red_highlight, 1.0, 0, result_display_img);

  // --- Focus Widget Logic ---
  if (widget_enabled_) {
    if (roi_.has_value()) {
      auto & roi = roi_.value();
      focus_widget_input = focus_widget_input(roi);
      RCLCPP_INFO(get_logger(), "ROI: x:%d, y:%d, w:%d, h:%d", roi.x, roi.y, roi.width, roi.height);
    }
    double current_focus_score = calculate_focus_score(focus_widget_input);

    focus_widget_->update(current_focus_score);

    double min_hist_score = focus_widget_->get_min_score();
    double max_hist_score = focus_widget_->get_max_score();

    RCLCPP_INFO(
      get_logger(), "Focus Score: %.2f (Min: %.2f, Max: %.2f)", current_focus_score, min_hist_score,
      max_hist_score);

    roi_selector_.draw(result_display_img);
    focus_widget_->draw(result_display_img);
  }

  cv::imshow(viz_window_name_, result_display_img);
  char key = cv::waitKey(1) & 0xFF;
  if (key == 'r') {
    RCLCPP_INFO(get_logger(), "Resetting Focus Peaking");
    roi_selector_.reset();
    focus_widget_->reset();
    roi_.reset();
  }
}

}  // namespace focus_peaking

// Register the node as a component
#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(focus_peaking::FocusPeaking)