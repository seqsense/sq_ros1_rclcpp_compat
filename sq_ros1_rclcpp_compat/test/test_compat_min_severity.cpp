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

#define ROSCONSOLE_MIN_SEVERITY ROSCONSOLE_SEVERITY_INFO

#include <gtest/gtest.h>
#include <ros/console.h>

#include "rclcpp/rclcpp.hpp"

TEST(CompatMinSeverity, CompilesOutBelowMinimum)
{
  ros::console::initialize();
  ASSERT_TRUE(ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Debug));
  ros::console::notifyLoggerLevelsChanged();

  auto logger = rclcpp::get_logger("test_logger");
  int evaluated = 0;
  RCLCPP_DEBUG(logger, "debug: %d", ++evaluated);
  RCLCPP_INFO(logger, "info: %d", ++evaluated);

  // Like ROS_DEBUG, the compiled-out call does not evaluate its arguments.
  EXPECT_EQ(evaluated, 1);
}
