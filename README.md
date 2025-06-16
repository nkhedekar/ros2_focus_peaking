# Focus Peaking ROS2

## Overview

This ROS 2 package assists in manually focusing lenses on machine vision cameras and evaluating image sharpness. It highlights sharp edges directly in the image display, allowing users to visually determine focus quality and identify which parts of the scene are in focus. An accompanying widget provides a quantitative focus quality score, calculated either from the full image or a user-defined Region of Interest (ROI) that can be selected interactively by clicking and dragging.

![demo image](images/focus_peaking_demo.png)

## Features
*   **Visual Focus Aid:** Highlights sharp edges in real-time.
*   **Quantitative Sharpness Score:** Provides a numerical metric for focus quality.
*   **Interactive ROI Selection:** Allows focusing on specific areas of interest.
*   **Focus History:** The sharpness score considers a history of frames (configurable via `focus_history_size` parameter) to provide a stable peak reading, mitigating temporary fluctuations (e.g., minor lighting changes).

## Video Demonstration

A full video demonstrating the setup process and the tool in action is available here:
[![Watch the video](https://img.youtube.com/vi/Wy3PcOUVC_Y/maxresdefault.jpg)](https://youtu.be/Wy3PcOUVC_Y)

## Usage

This section guides you through setting up your camera and using the focus peaking tool.

### Prerequisites

*   **Camera:** A camera with a manually adjustable focus ring (and preferably, aperture ring).
    *   *Example:* [Raspberry Pi High Quality Camera](https://www.raspberrypi.com/products/raspberry-pi-high-quality-camera/) with C/CS mount lenses.
*   **ROS 2 Drivers:** Functional ROS 2 drivers for your camera that publish raw image data (e.g., topics of type `sensor_msgs/Image`).
*   **Stable Mounting:** A steady mount for the camera to prevent movement during focusing.
*   **Textured Target:** An object or surface with sufficient texture or detail at the desired focusing distance.

### Setup and Launch

1.  **Mount Camera:** Securely mount your camera.
2.  **Connect and Verify Camera:**
    *   Connect the camera to your computer.
    *   Launch the camera's ROS 2 drivers.
    *   Confirm that raw images are being published by checking the available topics:
        ```bash
        ros2 topic list
        ```
        (Look for a topic like `/image_raw` or similar, depending on your camera driver).
3.  **Position Target:** Place your textured target object in the camera's field of view at the desired focusing distance.
4.  **Launch Focus Peaking Node:**
    Start the focus peaking node. You'll likely need to remap the `image` topic to your camera's raw image topic and adjust parameters as needed. See launch details in sections below.

### Focusing Procedure

1.  **Adjust Aperture (if available):** Set your lens aperture to the desired F-stop. A wider aperture (smaller F-number) results in a shallower depth of field, making critical focus more apparent but also harder to achieve.
2.  **Rough Focus & ROI Selection (Optional):**
    *   Begin adjusting the focus ring. You should see edges in the image start to be highlighted.
    *   If you want to focus on a specific area, click and drag on the image display window to draw a Region of Interest (ROI). The sharpness score will then be calculated only for this region.
3.  **Fine-Tune Focus:**
    *   Carefully adjust the focus ring back and forth.
    *   Observe the highlighted edges becoming sharper and more prominent in the areas you want in focus.
    *   Simultaneously, watch the sharpness indicator widget. Adjust the focus to maximize this score.
    *   The peak score is determined using the last `focus_history_size` frames. This helps find a stable maximum even with slight environmental variations. Continue adjusting until the indicator shows a clear peak.

### Important Considerations for Best Results

*   **Lighting:**
    *   **Steady Conditions:** Aim for consistent lighting. Fluctuations in brightness can affect the sharpness score.
    *   **Sufficient Light:** Poor lighting can lead to noisy images, and the Laplacian filter used for edge detection is sensitive to noise, potentially giving unstable sharpness readings.
*   **Camera Settings:**
    *   **Disable Auto Functions:** If possible, turn OFF auto-exposure and auto-gain on your camera.
    *   **Fixed Values:** Set fixed exposure and gain values. This ensures that changes in the sharpness metric are due to focus adjustments, not the camera's automatic adjustments to brightness.

### Example Setup

The image below shows a typical setup using a FLIR (Point Grey) Blackfly S USB machine vision camera with a C-mount lens (featuring manual aperture and focus rings). The camera is on a tripod, viewing various textured objects. Custom ROS 2 drivers are used, allowing manual control over exposure and gain settings, which are disabled for optimal focus peaking.

![FLIR USB camera with C-mount lens on a tripod, viewing textured objects for focus peaking.](images/camera_setup.png)

## Subscribed topics

**/image_raw** *(sensor_msgs::Image)*: Image topic published directly from the camera without any processing.

## Published topics

**/focus_metrics** *(focus_peaking_interfaces::msg::FocusMetrics)*: Focus quality metric calculated as variance of laplacian of the image. This could be further used to develop auto focus algorithms for the specific camera.

## Interfaces

### Messages

**focus_peaking_interfaces::msg::FocusMetrics**: Focus quality metric described in the [FocusMetrics.msg](focus_peaking_interfaces/msg/FocusMetrics.msg) file

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

