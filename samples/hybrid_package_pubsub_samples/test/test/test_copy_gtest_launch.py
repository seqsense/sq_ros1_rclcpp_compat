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

"""
Launch test that runs a C++ gtest executable with a publisher node.

This demonstrates the ROS 2 equivalent of rostest + gtest:
  - rostest launches nodes + gtest executable defined in .launch XML
  - launch_testing launches nodes + gtest executable defined in Python
"""

from typing import Tuple
import unittest

from launch import LaunchDescription
from launch.launch_description_entity import LaunchDescriptionEntity
from launch_ros.actions import Node
import launch_testing
from launch_testing.actions import ReadyToTest
import launch_testing.markers
from launch_testing.proc_info_handler import ActiveProcInfoHandler
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

    # C++ gtest executable built by ament_add_gtest_executable
    gtest_node = Node(
        package='hybrid_package_pubsub_samples',
        executable='test_copy_gtest_node',
        name='test_copy_gtest',
        output='screen',
    )

    return LaunchDescription([publisher_node, gtest_node, ReadyToTest()]), {
        'gtest_node': gtest_node
    }


class TestCopyGtest(unittest.TestCase):
    def test_gtest_pass(
        self, proc_info: ActiveProcInfoHandler, gtest_node: Node
    ) -> None:
        proc_info.assertWaitForShutdown(process=gtest_node, timeout=60.0)
        self.assertEqual(proc_info[gtest_node].returncode, 0)
