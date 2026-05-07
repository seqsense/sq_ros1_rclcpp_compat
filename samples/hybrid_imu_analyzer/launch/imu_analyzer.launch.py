from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description() -> LaunchDescription:
    return LaunchDescription(
        [
            Node(
                package='hybrid_imu_analyzer',
                executable='imu_analyzer_node_exec',
                name='imu_analyzer',
                output='screen',
            ),
        ]
    )
