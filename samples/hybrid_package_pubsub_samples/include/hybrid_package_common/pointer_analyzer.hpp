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

#ifndef HYBRID_PACKAGE_COMMON__POINTER_ANALYZER_HPP_
#define HYBRID_PACKAGE_COMMON__POINTER_ANALYZER_HPP_

#include <memory>

#include "hybrid_package_msgs/msg/stamped_pointer.hpp"
#include "rclcpp/rclcpp.hpp"

namespace hybrid_package_common
{

class PointerAnalyzer
{
public:
  explicit PointerAnalyzer(const rclcpp::Logger & logger);
  bool analyze(const std::shared_ptr<const hybrid_package_msgs::msg::StampedPointer> & msg);

private:
  rclcpp::Logger logger_;
};

}  // namespace hybrid_package_common

#endif  // HYBRID_PACKAGE_COMMON__POINTER_ANALYZER_HPP_
