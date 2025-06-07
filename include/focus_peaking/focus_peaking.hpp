#ifndef FOCUS_PEAKING__FOCUS_PEAKING_HPP_
#define FOCUS_PEAKING__FOCUS_PEAKING_HPP_

#include <opencv2/core/mat.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>

namespace focus_peaking
{
class FocusPeaking : public rclcpp::Node
{
public:
  explicit FocusPeaking(const rclcpp::NodeOptions & options);

  void image_callback(const sensor_msgs::msg::Image::ConstSharedPtr & msg);

  double calculate_focus_score(const cv::Mat & gray_image);

  void draw_focus_widget(
    cv::Mat & display_image, double current_score, double min_hist_score, double max_hist_score);

private:
  std::shared_ptr<rclcpp::Subscription<sensor_msgs::msg::Image>> image_sub_;

  std::string viz_window_name_;
  int64_t denoising_kernel_size_;
  int64_t edge_dilation_kernel_size_;
  bool widget_enabled_;
  double widget_width_ratio_;
  double widget_height_ratio_;
  int widget_margin_px_;
  size_t focus_history_size_;
  std::deque<double> focus_scores_history_;
};
}  // namespace focus_peaking

#endif  // FOCUS_PEAKING__FOCUS_PEAKING_HPP_