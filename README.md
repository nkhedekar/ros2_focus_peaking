# Focus Peaking ROS2

## Overview

This package is a helper for setting up lenses on machine vision cameras and testing image sharpness.
Sharp edges are highlighted in the viewing window which enables the user to determine the quality of focus and the target of focus in the scene.

## Subscribed topics

**/image_raw** *(sensor_msgs::Image)*: Image topic published directly from the camera without any processing.

## Parameters

**denoising_kernel_size** *(int64)*: Size of kernel used to apply gaussian blur on the image before edge detection. A larger value will denoise the image and only the sharpest edges will be visible. *(default: 3)*

**dilation_kernel_size** *(int64)*: Size of kernel used to dilate the edges for better visibility in the visualisation window. *(default: 3)*

## Build

To build the package run the following

```bash
colcon build --packages-up-to focus_peaking
```

## Run

As an independent node

```bash
# from ros2_ws
source install/setup.bash
ros2 run focus_peaking focus_peaking_node
```

From launch file

```bash
# from ros2_ws
source install/setup.bash
ros2 launch focus_peaking focus_peaking.launch.py
```

Launch demo with builtin laptop webcam.

- install ros2 v4l2-camera package
```bash
sudo apt update
sudo apt install ros-${ROS_DISTRO}-v4l2-camera
```
- launch the camera
```bash
# from ros2_ws
source install/setup.bash
ros2 run v4l2_camera v4l2_camera_node
```
- make sure the published topic of v4l2_camera matches that of the focus_peaking node (/image_raw).
