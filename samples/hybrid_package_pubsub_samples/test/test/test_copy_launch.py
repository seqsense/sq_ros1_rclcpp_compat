import unittest
from typing import Tuple

import launch_testing
import launch_testing.markers
import pytest
from launch import LaunchDescription
from launch.launch_description_entity import LaunchDescriptionEntity
from launch_ros.actions import Node
from launch_testing.actions import ReadyToTest


@pytest.mark.launch_test
@launch_testing.markers.keep_alive
def generate_test_description() -> Tuple[LaunchDescription, dict[str, LaunchDescriptionEntity]]:
    publisher_node = Node(
        package="hybrid_package_pubsub_samples",
        executable="pointer_publisher_node_exec",
        name="pointer_publisher",
        output="screen",
    )
    subscriber_node = Node(
        package="hybrid_package_pubsub_samples",
        executable="pointer_subscriber_node_exec",
        name="pointer_subscriber",
        output="screen",
    )

    return LaunchDescription(
        [publisher_node, subscriber_node, ReadyToTest()]
    ), {"subscriber_node": subscriber_node}


class TestCopy(unittest.TestCase):

    def test_copy_confirmed(
        self, proc_output: launch_testing.IoHandler, subscriber_node: Node
    ) -> None:
        proc_output.assertWaitFor(
            "RESULT:COPY_OK",
            process=subscriber_node,
            timeout=30.0,
        )
