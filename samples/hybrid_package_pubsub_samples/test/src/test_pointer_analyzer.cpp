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

// On now(): analyze() computes time_diff via Clock(RCL_ROS_TIME).now() - stamp; logged but not
// asserted. Messages are stamped just before analyze() to keep the subtraction well defined.
//
// On main(): ROS 2 ament_add_gtest links gtest_main automatically; ROS 1 tests link
// sq_ros1_rclcpp_compat_gtest_main (calls ros::Time::init()). No build-specific #ifdefs here.

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>

#include "hybrid_package_common/pointer_analyzer.hpp"
#include "hybrid_package_msgs/msg/stamped_pointer.hpp"
#include "rclcpp/rclcpp.hpp"

namespace
{

std::shared_ptr<hybrid_package_msgs::msg::StampedPointer> make_msg()
{
  auto msg = std::make_shared<hybrid_package_msgs::msg::StampedPointer>();
  msg->header.stamp = rclcpp::Clock(RCL_ROS_TIME).now();
  return msg;
}

}  // namespace

class PointerAnalyzerTest : public ::testing::Test
{
protected:
  hybrid_package_common::PointerAnalyzer analyzer_{rclcpp::get_logger("pointer_analyzer_test")};
};

TEST_F(PointerAnalyzerTest, MatchingPointerIsReportedAsZeroCopy)
{
  auto msg = make_msg();
  msg->pointer_value = reinterpret_cast<uint64_t>(msg.get());

  EXPECT_TRUE(analyzer_.analyze(msg));
}

TEST_F(PointerAnalyzerTest, MismatchingPointerIsReportedAsCopy)
{
  auto msg = make_msg();
  // Any value that cannot match msg.get() works; pick a recognisable one.
  msg->pointer_value = 0xDEADBEEFu;

  EXPECT_FALSE(analyzer_.analyze(msg));
}

TEST_F(PointerAnalyzerTest, DefaultZeroPointerValueIsReportedAsCopy)
{
  // Fresh message: pointer_value defaults to 0, msg.get() is non-zero,
  // so the two cannot match.
  auto msg = make_msg();
  ASSERT_EQ(msg->pointer_value, 0u);

  EXPECT_FALSE(analyzer_.analyze(msg));
}
