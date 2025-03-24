#include <memory>
#include <rclcpp/rclcpp.hpp>

#include "focus_peaking/focus_peaking.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<focus_peaking::FocusPeaking>();

  rclcpp::spin(node);
  rclcpp::shutdown();

  return 0;
}
