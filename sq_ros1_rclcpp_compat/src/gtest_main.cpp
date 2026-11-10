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
// catkin_add_gtest does not link gtest_main on Ubuntu; tests link this instead.
// ros::Time::init() is not called here but in gtest_ros_time_init.cpp, because
// this main() is not used where another main() is linked first (e.g. Alpine,
// where catkin_add_gtest links the system libgmock_main).
//
// On ROS 2, ament_add_gtest already supplies gtest_main, so this is not needed there.

#include <gtest/gtest.h>

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
