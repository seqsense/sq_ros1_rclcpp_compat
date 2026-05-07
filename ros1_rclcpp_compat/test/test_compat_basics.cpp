/*
 * Unit tests for ros1_rclcpp_compat basic APIs:
 * Logger, Clock, Time, Duration, logging macros.
 */

#include <gtest/gtest.h>
#include <ros/ros.h>

#include "rclcpp/rclcpp.hpp"

TEST(CompatLogger, GetLogger)
{
  auto logger = rclcpp::get_logger("test_logger");
  EXPECT_STREQ(logger.get_name(), "test_logger");
}

TEST(CompatLogger, LoggingMacros)
{
  auto logger = rclcpp::get_logger("test_logger");
  // Should not crash
  RCLCPP_INFO(logger, "info: %d", 1);
  RCLCPP_WARN(logger, "warn: %s", "hello");
  RCLCPP_ERROR(logger, "error: %.2f", 3.14);
  RCLCPP_DEBUG(logger, "debug: %d", 42);
}

TEST(CompatDuration, ConstructFromSecondsNanoseconds)
{
  rclcpp::Duration d(1, 500000000);
  EXPECT_DOUBLE_EQ(d.seconds(), 1.5);
  EXPECT_EQ(d.nanoseconds(), 1500000000LL);
}

TEST(CompatDuration, ConstructFromNanoseconds)
{
  rclcpp::Duration d(static_cast<int64_t>(2000000000LL));
  EXPECT_DOUBLE_EQ(d.seconds(), 2.0);
}

TEST(CompatDuration, FromSeconds)
{
  auto d = rclcpp::Duration::from_seconds(0.5);
  EXPECT_NEAR(d.seconds(), 0.5, 1e-9);
}

TEST(CompatDuration, FromRosDuration)
{
  ros::Duration ros_d(1, 250000000);
  rclcpp::Duration d(ros_d);
  EXPECT_DOUBLE_EQ(d.seconds(), 1.25);
}

TEST(CompatDuration, Arithmetic)
{
  rclcpp::Duration a(1, 0);
  rclcpp::Duration b(0, 500000000);
  auto sum = a + b;
  EXPECT_DOUBLE_EQ(sum.seconds(), 1.5);
  auto diff = a - b;
  EXPECT_DOUBLE_EQ(diff.seconds(), 0.5);
}

TEST(CompatDuration, Comparison)
{
  rclcpp::Duration a(1, 0);
  rclcpp::Duration b(2, 0);
  EXPECT_TRUE(a < b);
  EXPECT_TRUE(b > a);
  EXPECT_TRUE(a != b);
  EXPECT_TRUE(a == a);
}

TEST(CompatTime, ConstructFromRosTime)
{
  ros::Time ros_t(10, 500000000);
  rclcpp::Time t(ros_t);
  EXPECT_DOUBLE_EQ(t.seconds(), 10.5);
}

TEST(CompatTime, Arithmetic)
{
  rclcpp::Time t(10, 0, RCL_ROS_TIME);
  rclcpp::Duration d(2, 0);
  auto t2 = t + d;
  EXPECT_DOUBLE_EQ(t2.seconds(), 12.0);
  auto diff = t2 - t;
  EXPECT_DOUBLE_EQ(diff.seconds(), 2.0);
}

TEST(CompatClock, RosTimeNow)
{
  // RCL_ROS_TIME requires ros::init (handled in main)
  rclcpp::Clock clock(RCL_ROS_TIME);
  auto now = clock.now();
  EXPECT_GT(now.nanoseconds(), 0);
}

TEST(CompatClock, SystemTime)
{
  rclcpp::Clock clock(RCL_SYSTEM_TIME);
  auto now = clock.now();
  EXPECT_GT(now.nanoseconds(), 0);
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "test_compat_basics");
  ros::NodeHandle nh;  // Required for ros::Time::now()
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
