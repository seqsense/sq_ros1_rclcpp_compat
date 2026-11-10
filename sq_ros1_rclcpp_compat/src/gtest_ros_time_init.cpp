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

// Calls ros::Time::init() for ROS 1 tests linking sq_ros1_rclcpp_compat_gtest_main,
// so that rclcpp::Clock(RCL_ROS_TIME).now() (compat -> ros::Time::now()) works without
// extra setup.
//
// This runs as a static initializer instead of in main(), because main() in
// gtest_main.cpp is not used when another main() is linked first. The linker
// pulls an object out of a static library only to resolve an undefined symbol,
// so the exported link option -Wl,--undefined=sq_ros1_rclcpp_compat_gtest_ros_time_init
// makes it pull this object in either way.

#include <ros/time.h>

extern "C" {
extern const int sq_ros1_rclcpp_compat_gtest_ros_time_init;
const int sq_ros1_rclcpp_compat_gtest_ros_time_init = 0;
}

namespace
{
struct RosTimeInit
{
  RosTimeInit() { ros::Time::init(); }
};
const RosTimeInit ros_time_init;
}  // namespace
