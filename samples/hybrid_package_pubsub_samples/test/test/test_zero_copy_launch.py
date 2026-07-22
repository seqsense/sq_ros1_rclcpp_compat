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

from typing import Tuple
import unittest

from launch import LaunchDescription
from launch.launch_description_entity import LaunchDescriptionEntity
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode
import launch_testing
from launch_testing.actions import ReadyToTest
import launch_testing.markers
import pytest


@pytest.mark.launch_test
@launch_testing.markers.keep_alive
def generate_test_description() -> Tuple[LaunchDescription, dict[str, LaunchDescriptionEntity]]:
    container = ComposableNodeContainer(
        name='test_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container_mt',
        composable_node_descriptions=[
            ComposableNode(
                package='hybrid_package_pubsub_samples',
                plugin='hybrid_package_pubsub_samples::PointerPublisherNode',
                name='pointer_publisher',
                extra_arguments=[{'use_intra_process_comms': True}],
            ),
            ComposableNode(
                package='hybrid_package_pubsub_samples',
                plugin='hybrid_package_pubsub_samples::PointerSubscriberNode',
                name='pointer_subscriber_1',
                extra_arguments=[{'use_intra_process_comms': True}],
            ),
            ComposableNode(
                package='hybrid_package_pubsub_samples',
                plugin='hybrid_package_pubsub_samples::PointerSubscriberNode',
                name='pointer_subscriber_2',
                extra_arguments=[{'use_intra_process_comms': True}],
            ),
        ],
        output='screen',
    )

    return LaunchDescription([container, ReadyToTest()]), {'container': container}


class TestZeroCopy(unittest.TestCase):
    def test_zero_copy_subscriber_1(
        self, proc_output: launch_testing.IoHandler, container: ComposableNodeContainer
    ) -> None:
        proc_output.assertWaitFor(
            'RESULT:ZERO_COPY_OK [pointer_subscriber_1]',
            process=container,
            timeout=30.0,
        )

    def test_zero_copy_subscriber_2(
        self, proc_output: launch_testing.IoHandler, container: ComposableNodeContainer
    ) -> None:
        proc_output.assertWaitFor(
            'RESULT:ZERO_COPY_OK [pointer_subscriber_2]',
            process=container,
            timeout=30.0,
        )
