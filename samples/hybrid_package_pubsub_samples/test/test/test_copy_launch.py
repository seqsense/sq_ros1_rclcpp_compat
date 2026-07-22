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
from launch_ros.actions import Node
import launch_testing
from launch_testing.actions import ReadyToTest
import launch_testing.markers
import pytest


@pytest.mark.launch_test
@launch_testing.markers.keep_alive
def generate_test_description() -> Tuple[LaunchDescription, dict[str, LaunchDescriptionEntity]]:
    publisher_node = Node(
        package='hybrid_package_pubsub_samples',
        executable='pointer_publisher_node_exec',
        name='pointer_publisher',
        output='screen',
    )
    subscriber_node = Node(
        package='hybrid_package_pubsub_samples',
        executable='pointer_subscriber_node_exec',
        name='pointer_subscriber',
        output='screen',
    )

    return LaunchDescription(
        [publisher_node, subscriber_node, ReadyToTest()]
    ), {'subscriber_node': subscriber_node}


class TestCopy(unittest.TestCase):

    def test_copy_confirmed(
        self, proc_output: launch_testing.IoHandler, subscriber_node: Node
    ) -> None:
        proc_output.assertWaitFor(
            'RESULT:COPY_OK',
            process=subscriber_node,
            timeout=30.0,
        )
