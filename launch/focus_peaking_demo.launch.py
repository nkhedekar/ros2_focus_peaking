from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='focus_peaking',
            executable='focus_peaking_node',
            name='focus_peaking',
            output='screen',
            parameters=[
                {"denoising_kernel_size": 3},
                {"edge_dilation_kernel_size": 3},
            ]
        ),
        Node(
            package='usb_cam',
            executable='usb_cam_node_exe',
            name='webcam',
            parameters=[
                {"video_device": "/dev/video0"}
            ]
        )
    ])
