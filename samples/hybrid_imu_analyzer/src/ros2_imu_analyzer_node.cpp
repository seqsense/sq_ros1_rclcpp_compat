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

#include <memory>

#include "hybrid_imu_analyzer/imu_analyzer.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"

namespace hybrid_imu_analyzer
{

class ImuAnalyzerNode : public rclcpp::Node
{
public:
  explicit ImuAnalyzerNode(const rclcpp::NodeOptions & options)
  : rclcpp::Node("imu_analyzer", options), analyzer_(this->get_logger())
  {
    sub_ = this->create_subscription<sensor_msgs::msg::Imu>(
      "imu", 10, std::bind(&ImuAnalyzerNode::callback, this, std::placeholders::_1));
    RCLCPP_INFO(this->get_logger(), "ImuAnalyzerNode started");
  }

private:
  void callback(const sensor_msgs::msg::Imu::ConstSharedPtr & msg) { analyzer_.analyze(msg); }

  ImuAnalyzer analyzer_;
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr sub_;
};

}  // namespace hybrid_imu_analyzer

#include "rclcpp_components/register_node_macro.hpp"
RCLCPP_COMPONENTS_REGISTER_NODE(hybrid_imu_analyzer::ImuAnalyzerNode)
