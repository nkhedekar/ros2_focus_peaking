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
            package='v4l2_camera',
            executable='v4l2_camera_node',
            name='webcam',
            parameters=[
                {"video_device": "/dev/video0"}
            ]
        )
    ])
