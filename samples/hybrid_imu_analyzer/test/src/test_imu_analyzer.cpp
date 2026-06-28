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

// On now(): analyze() calls Clock(RCL_ROS_TIME).now() for latency; latency is logged but not
// asserted. Messages are stamped with the same clock just before analyze() to keep the
// subtraction well defined.
//
// On main(): ROS 2 ament_add_gtest links gtest_main automatically; ROS 1 tests link
// sq_ros1_rclcpp_compat_gtest_main (calls ros::Time::init()). No build-specific #ifdefs here.

#include <gtest/gtest.h>

#include <cmath>
#include <memory>

#include "hybrid_imu_analyzer/imu_analyzer.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"

namespace
{

constexpr double kEpsilon = 1e-9;

std::shared_ptr<sensor_msgs::msg::Imu> make_imu_msg(double qx, double qy, double qz, double qw)
{
  auto msg = std::make_shared<sensor_msgs::msg::Imu>();
  msg->orientation.x = qx;
  msg->orientation.y = qy;
  msg->orientation.z = qz;
  msg->orientation.w = qw;
  msg->header.stamp = rclcpp::Clock(RCL_ROS_TIME).now();
  return msg;
}

}  // namespace

class ImuAnalyzerTest : public ::testing::Test
{
protected:
  hybrid_imu_analyzer::ImuAnalyzer analyzer_{rclcpp::get_logger("imu_analyzer_test")};
};

TEST_F(ImuAnalyzerTest, IdentityQuaternionGivesZeroRpy)
{
  const auto rpy = analyzer_.analyze(make_imu_msg(0.0, 0.0, 0.0, 1.0));
  EXPECT_NEAR(rpy.roll, 0.0, kEpsilon);
  EXPECT_NEAR(rpy.pitch, 0.0, kEpsilon);
  EXPECT_NEAR(rpy.yaw, 0.0, kEpsilon);
}

TEST_F(ImuAnalyzerTest, RotationAroundXIsRoll)
{
  const double s = std::sin(M_PI / 4.0);
  const double c = std::cos(M_PI / 4.0);
  const auto rpy = analyzer_.analyze(make_imu_msg(s, 0.0, 0.0, c));
  EXPECT_NEAR(rpy.roll, M_PI / 2.0, kEpsilon);
  EXPECT_NEAR(rpy.pitch, 0.0, kEpsilon);
  EXPECT_NEAR(rpy.yaw, 0.0, kEpsilon);
}

TEST_F(ImuAnalyzerTest, RotationAroundYIsPitch)
{
  const double s = std::sin(M_PI / 4.0);
  const double c = std::cos(M_PI / 4.0);
  const auto rpy = analyzer_.analyze(make_imu_msg(0.0, s, 0.0, c));
  EXPECT_NEAR(rpy.roll, 0.0, kEpsilon);
  EXPECT_NEAR(rpy.pitch, M_PI / 2.0, kEpsilon);
  EXPECT_NEAR(rpy.yaw, 0.0, kEpsilon);
}

TEST_F(ImuAnalyzerTest, RotationAroundZIsYaw)
{
  const double s = std::sin(M_PI / 4.0);
  const double c = std::cos(M_PI / 4.0);
  const auto rpy = analyzer_.analyze(make_imu_msg(0.0, 0.0, s, c));
  EXPECT_NEAR(rpy.roll, 0.0, kEpsilon);
  EXPECT_NEAR(rpy.pitch, 0.0, kEpsilon);
  EXPECT_NEAR(rpy.yaw, M_PI / 2.0, kEpsilon);
}

TEST_F(ImuAnalyzerTest, GimbalLockPositivePitch)
{
  // |sin(pitch)| == 1 is the asin-clamp boundary; both branches give pi/2.
  const double s = std::sin(M_PI / 4.0);
  const double c = std::cos(M_PI / 4.0);
  const auto rpy = analyzer_.analyze(make_imu_msg(0.0, s, 0.0, c));
  EXPECT_NEAR(rpy.pitch, M_PI / 2.0, kEpsilon);
}

TEST_F(ImuAnalyzerTest, GimbalLockNegativePitch)
{
  const double s = std::sin(-M_PI / 4.0);
  const double c = std::cos(-M_PI / 4.0);
  const auto rpy = analyzer_.analyze(make_imu_msg(0.0, s, 0.0, c));
  EXPECT_NEAR(rpy.pitch, -M_PI / 2.0, kEpsilon);
}
