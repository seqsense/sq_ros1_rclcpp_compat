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
#include <ros/console.h>
#include <ros/ros.h>

#include <chrono>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "rclcpp/rclcpp.hpp"

namespace
{

class CapturingAppender : public ros::console::LogAppender
{
public:
  void log(ros::console::Level level, const char * str, const char *, const char *, int) override
  {
    messages.emplace_back(level, str);
  }

  std::vector<std::pair<ros::console::Level, std::string>> messages;
};

class CompatLogging : public ::testing::Test
{
protected:
  void SetUp() override { ros::console::register_appender(&appender_); }
  void TearDown() override { ros::console::deregister_appender(&appender_); }

  CapturingAppender appender_;
};

}  // namespace

TEST(CompatLogger, GetLogger)
{
  auto logger = rclcpp::get_logger("test_logger");
  EXPECT_STREQ(logger.get_name(), "test_logger");
}

TEST_F(CompatLogging, MacrosLogThroughRosconsole)
{
  auto logger = rclcpp::get_logger("test_logger");
  RCLCPP_INFO(logger, "info: %d", 1);
  RCLCPP_WARN(logger, "warn: %s", "hello");
  RCLCPP_ERROR(logger, "error: %.2f", 3.14);

  ASSERT_EQ(appender_.messages.size(), 3u);
  EXPECT_EQ(appender_.messages[0].first, ros::console::levels::Info);
  EXPECT_EQ(appender_.messages[0].second, "info: 1");
  EXPECT_EQ(appender_.messages[1].first, ros::console::levels::Warn);
  EXPECT_EQ(appender_.messages[1].second, "warn: hello");
  EXPECT_EQ(appender_.messages[2].first, ros::console::levels::Error);
  EXPECT_EQ(appender_.messages[2].second, "error: 3.14");
}

TEST_F(CompatLogging, DebugFollowsRosconsoleLevel)
{
  auto logger = rclcpp::get_logger("test_logger");
  RCLCPP_DEBUG(logger, "hidden");
  EXPECT_TRUE(appender_.messages.empty());

  ASSERT_TRUE(ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Debug));
  ros::console::notifyLoggerLevelsChanged();
  RCLCPP_DEBUG(logger, "shown: %d", 2);
  ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Info);
  ros::console::notifyLoggerLevelsChanged();

  ASSERT_EQ(appender_.messages.size(), 1u);
  EXPECT_EQ(appender_.messages[0].first, ros::console::levels::Debug);
  EXPECT_EQ(appender_.messages[0].second, "shown: 2");
}

TEST_F(CompatLogging, ThrottleLogsFirstCallAndAfterPeriod)
{
  auto logger = rclcpp::get_logger("test_logger");
  rclcpp::Clock clock(RCL_STEADY_TIME);
  for (int i = 0; i < 3; ++i) {
    if (i == 2) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    }
    RCLCPP_WARN_THROTTLE(logger, clock, 1000, "throttled: %d", i);
  }

  ASSERT_EQ(appender_.messages.size(), 2u);
  EXPECT_EQ(appender_.messages[0].first, ros::console::levels::Warn);
  EXPECT_EQ(appender_.messages[0].second, "throttled: 0");
  EXPECT_EQ(appender_.messages[1].second, "throttled: 2");
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

int main(int argc, char ** argv)
{
  ros::init(argc, argv, "test_compat_basics");
  ros::NodeHandle nh;  // Required for ros::Time::now()
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
