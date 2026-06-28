# Copyright 2026 SEQSENSE, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.actions import GroupAction
from launch.conditions import IfCondition
from launch.conditions import UnlessCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import ComposableNodeContainer
from launch_ros.actions import Node
from launch_ros.descriptions import ComposableNode


def generate_launch_description() -> LaunchDescription:
    use_composition = LaunchConfiguration('use_composition')
    use_intra = LaunchConfiguration('use_intra_process_comms')

    declare_use_composition = DeclareLaunchArgument(
        'use_composition',
        default_value='true',
        description='Use component container (true) or standalone nodes (false)',
    )
    declare_use_intra = DeclareLaunchArgument(
        'use_intra_process_comms',
        default_value='true',
        description='Enable intra-process communication (zero-copy when true)',
    )

    # --- Composition mode ---
    container_name = 'pubsub_container'

    load_composable_nodes = GroupAction(
        condition=IfCondition(use_composition),
        actions=[
            ComposableNodeContainer(
                name=container_name,
                namespace='',
                package='rclcpp_components',
                executable='component_container_mt',
                composable_node_descriptions=[
                    ComposableNode(
                        package='hybrid_package_pubsub_samples',
                        plugin='hybrid_package_pubsub_samples::PointerPublisherNode',
                        name='pointer_publisher',
                        extra_arguments=[{'use_intra_process_comms': use_intra}],
                    ),
                    ComposableNode(
                        package='hybrid_package_pubsub_samples',
                        plugin='hybrid_package_pubsub_samples::PointerSubscriberNode',
                        name='pointer_subscriber_1',
                        extra_arguments=[{'use_intra_process_comms': use_intra}],
                    ),
                    ComposableNode(
                        package='hybrid_package_pubsub_samples',
                        plugin='hybrid_package_pubsub_samples::PointerSubscriberNode',
                        name='pointer_subscriber_2',
                        extra_arguments=[{'use_intra_process_comms': use_intra}],
                    ),
                ],
                output='screen',
            ),
        ],
    )

    # --- Standalone mode ---
    load_nodes = GroupAction(
        condition=UnlessCondition(use_composition),
        actions=[
            Node(
                package='hybrid_package_pubsub_samples',
                executable='pointer_publisher_node_exec',
                name='pointer_publisher',
                output='screen',
            ),
            Node(
                package='hybrid_package_pubsub_samples',
                executable='pointer_subscriber_node_exec',
                name='pointer_subscriber_1',
                output='screen',
            ),
            Node(
                package='hybrid_package_pubsub_samples',
                executable='pointer_subscriber_node_exec',
                name='pointer_subscriber_2',
                output='screen',
            ),
        ],
    )

    return LaunchDescription(
        [
            declare_use_composition,
            declare_use_intra,
            load_composable_nodes,
            load_nodes,
        ]
    )
