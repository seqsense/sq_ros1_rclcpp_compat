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

#include <gtest/gtest.h>
#include <ros/time.h>

#include "rclcpp/rclcpp.hpp"

TEST(CompatClockUninitialized, RosTimeNowWithoutTimeInit)
{
  ASSERT_THROW(ros::Time::now(), ros::TimeNotInitializedException);

  const ros::WallTime before = ros::WallTime::now();
  const rclcpp::Time now = rclcpp::Clock(RCL_ROS_TIME).now();
  EXPECT_GE(now.nanoseconds(), static_cast<int64_t>(before.toNSec()));
  EXPECT_NO_THROW(ros::Time::now());
}

// Neither ros::init() nor ros::Time::init() is called, unlike
// sq_ros1_rclcpp_compat_gtest_main.
int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
