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

#include "hybrid_package_common/pointer_analyzer.hpp"

namespace hybrid_package_common
{

PointerAnalyzer::PointerAnalyzer(const rclcpp::Logger & logger) : logger_(logger) {}

bool PointerAnalyzer::analyze(
  const std::shared_ptr<const hybrid_package_msgs::msg::StampedPointer> & msg)
{
  const double time_diff_sec = (rclcpp::Clock(RCL_ROS_TIME).now() - msg->header.stamp).seconds();

  const uint64_t received_ptr = reinterpret_cast<uint64_t>(msg.get());
  const bool is_zero_copy = (msg->pointer_value == received_ptr);

  RCLCPP_INFO(
    logger_, "stored=0x%lx received=0x%lx zero_copy=%s time_diff=%.6fs", msg->pointer_value,
    received_ptr, is_zero_copy ? "true" : "false", time_diff_sec);

  return is_zero_copy;
}

}  // namespace hybrid_package_common
