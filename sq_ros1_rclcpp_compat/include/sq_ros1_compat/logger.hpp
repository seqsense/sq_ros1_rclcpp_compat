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

#ifndef SQ_ROS1_COMPAT__LOGGER_HPP_
#define SQ_ROS1_COMPAT__LOGGER_HPP_

#include <string>

#include "rclcpp/logger.hpp"

namespace sq_ros1_compat
{

inline rclcpp::Logger get_logger(const std::string & name) { return rclcpp::get_logger(name); }

}  // namespace sq_ros1_compat

#endif  // SQ_ROS1_COMPAT__LOGGER_HPP_
