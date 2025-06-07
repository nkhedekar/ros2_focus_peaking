#include "focus_peaking/focus_peaking.hpp"  // Or your actual header file

#include <algorithm>  // For std::min_element, std::max_element, std::min, std::max
#include <cv_bridge/cv_bridge.hpp>
#include <deque>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

namespace focus_peaking
{

// (create_param_descriptor function remains the same)
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
  widget_width_ratio_(0.015),
  widget_height_ratio_(0.2),
  widget_margin_px_(20),
  focus_history_size_(200)
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
    "widget_height_ratio", widget_height_ratio_,
    create_param_descriptor(
      "Widget height as a ratio of image height (0.0 to 1.0)",
      rclcpp::ParameterType::PARAMETER_DOUBLE));

  declare_parameter(
    "widget_margin_px", widget_margin_px_,
    create_param_descriptor(
      "Widget margin from image border in pixels. Must be non-negative.",
      rclcpp::ParameterType::PARAMETER_INTEGER));

  // For focus_history_size, ROS parameter system uses int64, while member is size_t
  // So we declare with an int representation of the default.
  declare_parameter(
    "focus_history_size", static_cast<int64_t>(focus_history_size_),
    create_param_descriptor(
      "Number of frames for focus score history (min 1)",
      rclcpp::ParameterType::PARAMETER_INTEGER));

  denoising_kernel_size_ = get_parameter("denoising_kernel_size").as_int();
  edge_dilation_kernel_size_ = get_parameter("edge_dilation_kernel_size").as_int();
  widget_enabled_ = get_parameter("widget_enabled").as_bool();
  // START: Get modified widget dimension parameter values
  widget_width_ratio_ = get_parameter("widget_width_ratio").as_double();
  widget_height_ratio_ = get_parameter("widget_height_ratio").as_double();
  // END: Get modified widget dimension parameter values

  widget_margin_px_ = get_parameter("widget_margin_px").as_int();
  focus_history_size_ = get_parameter("focus_history_size").as_int();

  // LOG all parameter values
  RCLCPP_INFO(get_logger(), "denoising_kernel_size: %ld", denoising_kernel_size_);
  RCLCPP_INFO(get_logger(), "edge_dilation_kernel_size: %ld", edge_dilation_kernel_size_);
  RCLCPP_INFO(get_logger(), "widget_enabled: %s", widget_enabled_ ? "true" : "false");
  RCLCPP_INFO(get_logger(), "widget_width_ratio: %lf", widget_width_ratio_);
  RCLCPP_INFO(get_logger(), "widget_height_ratio: %lf", widget_height_ratio_);
  RCLCPP_INFO(get_logger(), "widget_margin_px: %d", widget_margin_px_);
  RCLCPP_INFO(get_logger(), "focus_history_size: %ld", focus_history_size_);

  image_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "/image_raw", 5,
    [&](const sensor_msgs::msg::Image::ConstSharedPtr & msg) { image_callback(msg); });

  cv::namedWindow(viz_window_name_, cv::WINDOW_NORMAL);
  cv::resizeWindow(viz_window_name_, 800, 600);

  RCLCPP_INFO(get_logger(), "FocusPeaking initialized with Laplacian Variance for widget.");
}

// START: New function for calculating focus score (Laplacian Variance)
double FocusPeaking::calculate_focus_score(const cv::Mat & gray_image)
{
  cv::Mat laplacian_img;
  // CV_64F for output depth to avoid overflow/clipping with Laplacian, then normalize for variance
  cv::Laplacian(gray_image, laplacian_img, CV_64F);

  cv::Scalar mean, stddev;
  cv::meanStdDev(laplacian_img, mean, stddev);

  // Variance is the square of the standard deviation
  return stddev.val[0] * stddev.val[0];
}
// END: New function for calculating focus score

// START: New function for drawing the widget
void FocusPeaking::draw_focus_widget(
  cv::Mat & display_image, double current_score, double min_hist_score, double max_hist_score)
{
  int img_h = display_image.rows;
  int img_w = display_image.cols;

  // Calculate actual widget dimensions based on ratios
  int widget_actual_w_px = static_cast<int>(img_w * widget_width_ratio_);
  int widget_actual_h_px = static_cast<int>(img_h * widget_height_ratio_);

  // Ensure minimum dimensions for visibility
  widget_actual_w_px = std::max(10, widget_actual_w_px);  // Min 10px width
  widget_actual_h_px = std::max(20, widget_actual_h_px);  // Min 20px height

  // Top-right corner for the widget's background
  cv::Point widget_tl(img_w - widget_actual_w_px - widget_margin_px_, widget_margin_px_);
  cv::Point widget_br(img_w - widget_margin_px_, widget_margin_px_ + widget_actual_h_px);
  cv::Rect widget_bg_rect(widget_tl, widget_br);

  // Draw white background for the widget
  cv::rectangle(display_image, widget_bg_rect, cv::Scalar(255, 255, 255), cv::FILLED);
  // Draw black border for the widget
  cv::rectangle(display_image, widget_bg_rect, cv::Scalar(0, 0, 0), 2);

  // Draw the red focus indicator line
  if (
    max_hist_score > min_hist_score &&
    (max_hist_score - min_hist_score) > 1e-6) {  // Avoid division by zero or tiny differences
    double normalized_score = (current_score - min_hist_score) / (max_hist_score - min_hist_score);
    normalized_score = std::max(0.0, std::min(1.0, normalized_score));  // Clamp to [0, 1]

    // Y-coordinate for the line: top of widget for score=1.0, bottom for score=0.0
    int line_y = widget_tl.y + static_cast<int>((1.0 - normalized_score) * widget_actual_h_px);

    cv::line(
      display_image, cv::Point(widget_tl.x, line_y), cv::Point(widget_br.x, line_y),
      cv::Scalar(0, 0, 255), 3);  // Red line, thickness 2
  } else {
    // If min_hist_score is very close to max_hist_score (e.g., first few frames, static scene, or history_size=1)
    // Draw line in the middle with a different color (e.g., gray) to indicate neutral/undetermined
    int line_y = widget_tl.y + widget_actual_h_px / 2;
    cv::line(
      display_image, cv::Point(widget_tl.x, line_y), cv::Point(widget_br.x, line_y),
      cv::Scalar(128, 128, 128), 2);  // Gray line
  }
}
// END: New function for drawing the widget

void FocusPeaking::image_callback(const sensor_msgs::msg::Image::ConstSharedPtr & msg)
{
  RCLCPP_DEBUG(get_logger(), "Image received, processing...");

  cv_bridge::CvImagePtr cv_ptr;
  try {
    // Use toCvCopy to get a modifiable image if you draw directly on 'image'
    // or toCvShare if you only read from it before creating result_display_img
    cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
  } catch (cv_bridge::Exception & e) {
    RCLCPP_ERROR(get_logger(), "cv_bridge exception: %s", e.what());
    return;
  }
  cv::Mat image = cv_ptr->image;
  cv::Mat result_display_img = image.clone();  // Work on a copy for drawing

  // --- Focus Peaking Visualization (Canny edges) ---
  cv::Mat gray_for_peaking, edges_for_peaking;
  cv::cvtColor(image, gray_for_peaking, cv::COLOR_BGR2GRAY);
  // Denoising for Canny
  cv::GaussianBlur(
    gray_for_peaking, gray_for_peaking, cv::Size(denoising_kernel_size_, denoising_kernel_size_),
    0);
  cv::Canny(
    gray_for_peaking, edges_for_peaking, 50, 150);  // Keep Canny thresholds or make them params

  cv::Mat dilation_kernel = cv::getStructuringElement(
    cv::MORPH_DILATE, cv::Size(edge_dilation_kernel_size_, edge_dilation_kernel_size_));
  cv::dilate(edges_for_peaking, edges_for_peaking, dilation_kernel);

  cv::Mat red_highlight = cv::Mat::zeros(image.size(), image.type());
  red_highlight.setTo(cv::Scalar(0, 0, 255), edges_for_peaking);
  cv::addWeighted(result_display_img, 1.0, red_highlight, 1.0, 0, result_display_img);

  // --- Focus Widget Logic (Laplacian Variance) ---
  if (widget_enabled_) {
    cv::Mat gray_for_laplacian;
    cv::cvtColor(image, gray_for_laplacian, cv::COLOR_BGR2GRAY);
    // Denoising before Laplacian is also generally good
    cv::GaussianBlur(
      gray_for_laplacian, gray_for_laplacian,
      cv::Size(denoising_kernel_size_, denoising_kernel_size_), 0);

    double current_focus_score = calculate_focus_score(gray_for_laplacian);

    focus_scores_history_.push_back(current_focus_score);
    while (focus_scores_history_.size() > focus_history_size_) {
      focus_scores_history_.pop_front();
    }

    double min_hist_score = current_focus_score;
    double max_hist_score = current_focus_score;

    if (!focus_scores_history_.empty()) {
      min_hist_score =
        *std::min_element(focus_scores_history_.begin(), focus_scores_history_.end());
      max_hist_score =
        *std::max_element(focus_scores_history_.begin(), focus_scores_history_.end());
    }
    // For debugging:
    RCLCPP_INFO(
      get_logger(), "Focus Score: %.2f (Min: %.2f, Max: %.2f)", current_focus_score, min_hist_score,
      max_hist_score);

    draw_focus_widget(result_display_img, current_focus_score, min_hist_score, max_hist_score);
  }

  cv::imshow(viz_window_name_, result_display_img);
  cv::waitKey(1);
}

}  // namespace focus_peaking

// Register the node as a component
#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(focus_peaking::FocusPeaking)