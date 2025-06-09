from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription(
        [
            Node(
                package="focus_peaking",
                executable="focus_peaking_node",
                name="focus_peaking",
                prefix=["xterm -fa Monospace -fs 14 -e gdb -ex run --args"],
                output="screen",
                parameters=[
                    {"denoising_kernel_size": 3},  # Default: 3
                    {"edge_dilation_kernel_size": 3},  # Default: 3
                    {"widget_enabled": True},  # Default: True
                    {"widget_width_ratio": 0.04},  # Default: 0.04
                    {"focus_history_size": 200},  # Default: 200
                ],
            )
        ]
    )
