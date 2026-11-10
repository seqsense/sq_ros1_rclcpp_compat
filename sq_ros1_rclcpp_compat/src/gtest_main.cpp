// Copyright 2026 SEQSENSE, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// gtest main for ROS 1 tests using sq_ros1_rclcpp_compat.
//
// catkin_add_gtest does not link gtest_main; tests link this instead.
// Also calls ros::Time::init() before RUN_ALL_TESTS so Clock(RCL_ROS_TIME).now() works.
//
// Tests link with:
//   target_link_libraries(<test> <logic_lib> sq_ros1_rclcpp_compat_gtest_main
//                         ${catkin_LIBRARIES})
//
// On ROS 2, ament_add_gtest already supplies gtest_main, so this is not needed there.

#include <gtest/gtest.h>
#include <ros/time.h>

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  ros::Time::init();
  return RUN_ALL_TESTS();
}
