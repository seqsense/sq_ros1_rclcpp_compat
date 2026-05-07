// Unit tests for hybrid_imu_analyzer::ImuAnalyzer.
//
// On now() in unit tests
// ----------------------
// ImuAnalyzer::analyze(msg) calls rclcpp::Clock(RCL_ROS_TIME).now() inside
// to compute the message latency. The test cannot control that clock, so
// the latency is wall-clock dependent and is deliberately NOT asserted.
//
// To keep the internal subtraction well defined (small, non-negative), we
// stamp the message with the same clock right before calling analyze().
// Tests then check only the deterministic conversion math (quaternion ->
// roll/pitch/yaw).
//
// The required main() differs between builds: on ROS 2 ament_add_gtest
// auto-links gtest_main, so this file needs no main(); on ROS 1 the test
// links sq_ros1_rclcpp_compat_gtest_main (provided by sq_ros1_rclcpp_compat),
// which supplies a main() that calls ros::Time::init() before
// RUN_ALL_TESTS. Either way this source file stays free of build-specific
// preprocessor branches.
//
// If a future test needs a *deterministic* latency value, the right move
// is to make the clock injectable on ImuAnalyzer (constructor takes a
// rclcpp::Clock::SharedPtr) or to extend the analyze() signature to take
// `now` as an argument. We keep the sample API minimal here on purpose;
// the test focuses on the math that benefits from gtest coverage.

#include "hybrid_imu_analyzer/imu_analyzer.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"

namespace
{

constexpr double kEpsilon = 1e-9;

// Build an Imu message with the given quaternion, stamped with the same
// clock the analyzer uses internally.
//
// We construct the SharedPtr from a raw `new` rather than std::make_shared
// because ROS 1's typedef is boost::shared_ptr while ROS 2's is
// std::shared_ptr; the raw-pointer ctor is valid for both.
sensor_msgs::msg::Imu::SharedPtr make_imu_msg(
  double qx, double qy, double qz, double qw)
{
  sensor_msgs::msg::Imu::SharedPtr msg(new sensor_msgs::msg::Imu);
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
  hybrid_imu_analyzer::ImuAnalyzer analyzer_{
    rclcpp::get_logger("imu_analyzer_test")};
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
