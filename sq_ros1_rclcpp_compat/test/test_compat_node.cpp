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

// Run via: rostest sq_ros1_rclcpp_compat test_compat_node.test --text

#include <gtest/gtest.h>
#include <ros/ros.h>

#include "rclcpp/rclcpp.hpp"

TEST(CompatNode, DeclareParameterWithDefault)
{
  rclcpp::Node node("test_node");
  const int val = node.declare_parameter("nonexistent_param", 42);
  EXPECT_EQ(val, 42);
}

TEST(CompatNode, DeclareParameterFromServer)
{
  // test_preloaded is set in the .test launch file
  rclcpp::Node node("test_node");
  const int val = node.declare_parameter("test_preloaded", 0);
  EXPECT_EQ(val, 99);
}

TEST(CompatNode, DeclareParameterDouble)
{
  rclcpp::Node node("test_node");
  const double val = node.declare_parameter("double_param", 3.14);
  EXPECT_DOUBLE_EQ(val, 3.14);
}

TEST(CompatNode, DeclareParameterBool)
{
  rclcpp::Node node("test_node");
  const bool val = node.declare_parameter("bool_param", true);
  EXPECT_TRUE(val);
}

TEST(CompatNode, DeclareParameterString)
{
  rclcpp::Node node("test_node");
  const std::string val = node.declare_parameter<std::string>("string_param", "hello");
  EXPECT_EQ(val, "hello");
}

TEST(CompatNode, DeclareParameterDoubleArray)
{
  // test_array is set in the .test launch file
  rclcpp::Node node("test_node");
  const auto val = node.declare_parameter<std::vector<double>>("test_array");
  ASSERT_EQ(val.size(), 3u);
  EXPECT_DOUBLE_EQ(val[0], 1.0);
  EXPECT_DOUBLE_EQ(val[1], 2.0);
  EXPECT_DOUBLE_EQ(val[2], 3.0);
}

TEST(CompatNode, DeclareParameterWithoutDefaultThrows)
{
  rclcpp::Node node("test_node");
  EXPECT_THROW(node.declare_parameter<int>("nonexistent_no_default"), std::runtime_error);
}

TEST(CompatNode, GetParameterAfterDeclare)
{
  rclcpp::Node node("test_node");
  node.declare_parameter("my_int", 42);

  int val = 0;
  EXPECT_TRUE(node.get_parameter("my_int", val));
  EXPECT_EQ(val, 42);
}

TEST(CompatNode, GetParameterObject)
{
  rclcpp::Node node("test_node");
  node.declare_parameter("my_double", 2.718);

  rclcpp::Parameter param;
  EXPECT_TRUE(node.get_parameter("my_double", param));
  EXPECT_EQ(param.get_name(), "my_double");
  EXPECT_DOUBLE_EQ(param.as_double(), 2.718);
}

TEST(CompatNode, GetParameterUndeclaredReturnsFalse)
{
  rclcpp::Node node("test_node");
  int val = 0;
  EXPECT_FALSE(node.get_parameter("never_declared", val));
}

TEST(CompatNode, GetLogger)
{
  rclcpp::Node node("my_node");
  auto logger = node.get_logger();
  RCLCPP_INFO(logger, "test message from compat node: %d", 42);
}

TEST(CompatNode, GetName)
{
  rclcpp::Node node("my_test_node");
  EXPECT_EQ(node.get_name(), "my_test_node");
}

TEST(CompatNode, Now)
{
  rclcpp::Node node("test_node");
  auto t = node.now();
  EXPECT_GT(t.nanoseconds(), 0);
}

int main(int argc, char ** argv)
{
  ros::init(argc, argv, "test_compat_node");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
