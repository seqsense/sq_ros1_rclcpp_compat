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

#include "hybrid_package_common/pointer_analyzer.hpp"
#include "hybrid_package_msgs/msg/stamped_pointer.hpp"
#include "rclcpp/rclcpp.hpp"

namespace hybrid_package_pubsub_samples
{
class PointerSubscriberNode : public rclcpp::Node
{
public:
  explicit PointerSubscriberNode(const rclcpp::NodeOptions & options)
  : rclcpp::Node("pointer_subscriber", options), analyzer_(this->get_logger())
  {
    subscription_ = this->create_subscription<hybrid_package_msgs::msg::StampedPointer>(
      "stamped_pointer", 10,
      std::bind(&PointerSubscriberNode::callback, this, std::placeholders::_1));
    RCLCPP_INFO(this->get_logger(), "PointerSubscriberNode started");
  }

private:
  void callback(const hybrid_package_msgs::msg::StampedPointer::ConstSharedPtr & msg)
  {
    const bool is_zero_copy = analyzer_.analyze(msg);
    RCLCPP_INFO(
      this->get_logger(), "%s [%s]", is_zero_copy ? "RESULT:ZERO_COPY_OK" : "RESULT:COPY_OK",
      this->get_name());
  }

  hybrid_package_common::PointerAnalyzer analyzer_;
  rclcpp::Subscription<hybrid_package_msgs::msg::StampedPointer>::SharedPtr subscription_;
};
}  // namespace hybrid_package_pubsub_samples

#include "rclcpp_components/register_node_macro.hpp"
RCLCPP_COMPONENTS_REGISTER_NODE(hybrid_package_pubsub_samples::PointerSubscriberNode)
