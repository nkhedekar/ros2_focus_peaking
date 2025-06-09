# Focus Peaking ROS2

## Overview

This package is a helper for setting up lenses on machine vision cameras and testing image sharpness.
Sharp edges are highlighted in the viewing window which enables the user to determine the quality of focus and the target of focus in the scene.

## Subscribed topics

**/image_raw** *(sensor_msgs::Image)*: Image topic published directly from the camera without any processing.

## Parameters

**denoising_kernel_size** *(int)*: Size of gaussian blur kernel used for denoising. Must be odd and positive. *(default: 3)*

**edge_dilation_kernel_size** *(int)*: Size of kernel used to dilate detected edges for peaking. Must be positive. *(default: 3)*

**widget_enabled** *(bool)*: Enable focus quality widget. *(default: true)*

**widget_width_ratio** *(double)*: Widget width as a ratio of image width (0.0 to 1.0). *(default: 0.04)*

**focus_history_size** *(int)*: Number of frames for focus score history (min 1). *(default: 200)*

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
> __NOTE__: This would only show the image with the edges and widget but no lens adjustment is available so cannot actually test the capabilities. The user needs a camera with adjustable manual focus and/or aperture control.

- install ros2 usb_cam [drivers](https://github.com/ros-drivers/usb_cam)
```bash
sudo apt update
sudo apt install ros-${ROS_DISTRO}-usb-cam
```
- launch the demo
```bash
# from ros2_ws
source install/setup.bash
ros2 launch focus_peaking focus_peaking_demo.launch.py
```

