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
#include <rosgraph_msgs/Log.h>

#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"

namespace
{

// Checks what reaches /rosout rather than registering a rosconsole appender:
// on Noetic, deregister_appender() double-frees the appender and holds only
// one, so it would also detach roscpp's own /rosout appender.
class CompatLogging : public ::testing::Test
{
protected:
  void SetUp() override
  {
    sub_ = nh_.subscribe("/rosout", 100, &CompatLogging::onLog, this);
    const ros::WallTime deadline = ros::WallTime::now() + ros::WallDuration(10.0);
    while (sub_.getNumPublishers() == 0 && ros::WallTime::now() < deadline) {
      ros::WallDuration(0.01).sleep();
    }
    ASSERT_GT(sub_.getNumPublishers(), 0u);
  }

  void waitForMessages(size_t count)
  {
    const ros::WallTime deadline = ros::WallTime::now() + ros::WallDuration(10.0);
    while (messages_.size() < count && ros::WallTime::now() < deadline) {
      ros::spinOnce();
      ros::WallDuration(0.01).sleep();
    }
  }

  std::vector<rosgraph_msgs::Log> messages_;

private:
  void onLog(const rosgraph_msgs::Log::ConstPtr & msg)
  {
    if (
      msg->name == ros::this_node::getName() &&
      msg->file.find("test_compat_basics") != std::string::npos) {
      messages_.push_back(*msg);
    }
  }

  ros::NodeHandle nh_;
  ros::Subscriber sub_;
};

}  // namespace

TEST(CompatLogger, GetLogger)
{
  auto logger = rclcpp::get_logger("test_logger");
  EXPECT_STREQ(logger.get_name(), "test_logger");
}

TEST_F(CompatLogging, MacrosLogToRosout)
{
  auto logger = rclcpp::get_logger("test_logger");
  RCLCPP_INFO(logger, "info: %d", 1);
  RCLCPP_WARN(logger, "warn: %s", "hello");
  RCLCPP_ERROR(logger, "error: %.2f", 3.14);
  waitForMessages(3);

  ASSERT_EQ(messages_.size(), 3u);
  EXPECT_EQ(messages_[0].level, rosgraph_msgs::Log::INFO);
  EXPECT_EQ(messages_[0].msg, "info: 1");
  EXPECT_EQ(messages_[1].level, rosgraph_msgs::Log::WARN);
  EXPECT_EQ(messages_[1].msg, "warn: hello");
  EXPECT_EQ(messages_[2].level, rosgraph_msgs::Log::ERROR);
  EXPECT_EQ(messages_[2].msg, "error: 3.14");
}

TEST_F(CompatLogging, DebugFollowsRosconsoleLevel)
{
  auto logger = rclcpp::get_logger("test_logger");
  RCLCPP_DEBUG(logger, "hidden");

  ASSERT_TRUE(ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Debug));
  ros::console::notifyLoggerLevelsChanged();
  RCLCPP_DEBUG(logger, "shown: %d", 2);
  ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Info);
  ros::console::notifyLoggerLevelsChanged();
  waitForMessages(1);

  // /rosout keeps the order, so a published "hidden" would come first.
  ASSERT_EQ(messages_.size(), 1u);
  EXPECT_EQ(messages_[0].level, rosgraph_msgs::Log::DEBUG);
  EXPECT_EQ(messages_[0].msg, "shown: 2");
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
