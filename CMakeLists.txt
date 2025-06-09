cmake_minimum_required(VERSION 3.8)
project(focus_peaking)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)

add_library(focus_peaking_component SHARED
  src/focus_peaking.cpp
  src/focus_widget.cpp
  src/roi_selector.cpp
)
target_include_directories(focus_peaking_component PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

target_include_directories(focus_peaking_component SYSTEM PUBLIC
  ${OpenCV_INCLUDE_DIRS}
)

set(focus_peaking_dependencies
  rclcpp
  rclcpp_components
  sensor_msgs
  cv_bridge
)

ament_target_dependencies(focus_peaking_component
  ${focus_peaking_dependencies}
)

target_link_libraries(focus_peaking_component
 ${OpenCV_LIBS}
)

rclcpp_components_register_node(
  focus_peaking_component
  PLUGIN "focus_peaking::FocusPeaking"
  EXECUTABLE focus_peaking_node
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_uncrustify_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()

  # find_package(ament_cmake_gtest REQUIRED)
  # add_subdirectory(test/gtests)
endif()

ament_export_targets(export_focus_peaking_component)
install(TARGETS focus_peaking_component
  EXPORT export_focus_peaking_component
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}/
)

# install(DIRECTORY
#   config
#   DESTINATION share/${PROJECT_NAME}/
# )

ament_package()
