# Focus Peaking ROS2

## Overview

This package is a helper for setting up lenses on machine vision cameras and testing image sharpness.
Sharp edges are highlighted in the viewing window, enabling the user to determine the quality of focus and the target of focus in the scene.
A widget on the right side of the image provides a quantitative measure of focus quality. The focus score for this widget can be calculated from the full image or a user-defined Region of Interest (ROI), which can be selected interactively.

## Subscribed topics

**/image_raw** *(sensor_msgs::Image)*: Image topic published directly from the camera without any processing.

## Parameters

**denoising_kernel_size** *(int)*: Size of gaussian blur kernel used for denoising. Must be odd and positive. *(default: 3)*

**edge_dilation_kernel_size** *(int)*: Size of kernel used to dilate detected edges for peaking. Must be positive. *(default: 3)*

**widget_enabled** *(bool)*: Enable focus quality widget. *(default: true)*

**widget_width_ratio** *(double)*: Widget width as a ratio of image width (0.0 to 1.0). *(default: 0.04)*

**focus_history_size** *(int)*: Number of frames for focus score history (min 1). *(default: 200)*

## Controls

The main display window allows for interactive control when it has focus:

*   **Select Region of Interest (ROI) for Focus Metric:**
    *   **How:** Click and drag with the **left mouse button** on the main image area
    *   **Effect:** A rectangle will be drawn on the image indicating the selected ROI. The focus score for the widget will then be calculated for the ROI only. This allows you to target focusing for a specific part of the scene.
    * Selecting a new ROI will replace the previous ROI.
    * The visual focus peaking (red edge highlights) will continue to be applied to the full image, regardless of the ROI selection for the metric.

*   **Reset Region of Interest (ROI):**
    * **How:** Press the **'r' key** while the main display window is active.
    * **Effect:** The selected ROI is cleared from the display. The focus score calculation will revert to using the full image until a new ROI is selected.

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

1. Ensure you have the usb_cam ROS 2 package installed
```bash
sudo apt update
sudo apt install ros-${ROS_DISTRO}-usb-cam
```
2. Launch the demo
```bash
# from ros2_ws
source install/setup.bash
ros2 launch focus_peaking focus_peaking_demo.launch.py
```

> __NOTE:__ The demo with a standard webcam will show the focus peaking highlights and the widget, but since most webcams have fixed focus or very limited autofocus, you won't be able to manually adjust the lens to test the full capabilities of this tool. This tool is most effective with a camera that has a manually adjustable focus ring and/or aperture control.

