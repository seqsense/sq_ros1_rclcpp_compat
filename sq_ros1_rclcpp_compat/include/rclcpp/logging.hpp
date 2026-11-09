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

#ifndef RCLCPP__LOGGING_HPP_
#define RCLCPP__LOGGING_HPP_

#include <ros/console.h>

#include <chrono>
#include <type_traits>

#include "rclcpp/logger.hpp"

// Each macro expands to the matching ROS_* macro, so it behaves exactly as a
// ROS_* call in the same package: it reaches /rosout, WARN and above go to
// stderr, the level follows rosconsole config and the node's set_logger_level
// service, and ROSCONSOLE_MIN_SEVERITY compiles it out.
//
// The rosconsole logger is therefore ROSCONSOLE_DEFAULT_NAME rather than one
// named after the rclcpp::Logger. rosconsole binds a logger name to each call
// site the first time it runs, so two instances logging from the same line
// under different names would both end up under whichever name came first.
#define RCLCPP_COMPAT_LOG_(ros_macro, logger, ...)                                             \
  do {                                                                                         \
    static_assert(                                                                             \
      std::is_same<                                                                            \
        typename std::remove_cv<typename std::remove_reference<decltype(logger)>::type>::type, \
        ::rclcpp::Logger>::value,                                                              \
      "First argument to logging macros must be an rclcpp::Logger");                           \
    ros_macro(__VA_ARGS__);                                                                    \
  } while (0)

#define RCLCPP_DEBUG(logger, ...) RCLCPP_COMPAT_LOG_(ROS_DEBUG, logger, __VA_ARGS__)
#define RCLCPP_INFO(logger, ...) RCLCPP_COMPAT_LOG_(ROS_INFO, logger, __VA_ARGS__)
#define RCLCPP_WARN(logger, ...) RCLCPP_COMPAT_LOG_(ROS_WARN, logger, __VA_ARGS__)
#define RCLCPP_ERROR(logger, ...) RCLCPP_COMPAT_LOG_(ROS_ERROR, logger, __VA_ARGS__)

// THROTTLE variants — clock argument is accepted but ignored; uses wall-clock throttling.
// period_ms is in milliseconds to match ROS 2 API.
#define RCLCPP_DEBUG_THROTTLE(logger, clock, period_ms, ...)                        \
  do {                                                                              \
    static auto _rclcpp_compat_last = std::chrono::steady_clock::time_point::min(); \
    auto _rclcpp_compat_now = std::chrono::steady_clock::now();                     \
    if (                                                                            \
      std::chrono::duration_cast<std::chrono::milliseconds>(                        \
        _rclcpp_compat_now - _rclcpp_compat_last)                                   \
        .count() >= (period_ms)) {                                                  \
      _rclcpp_compat_last = _rclcpp_compat_now;                                     \
      RCLCPP_DEBUG(logger, __VA_ARGS__);                                            \
    }                                                                               \
  } while (0)

#define RCLCPP_INFO_THROTTLE(logger, clock, period_ms, ...)                         \
  do {                                                                              \
    static auto _rclcpp_compat_last = std::chrono::steady_clock::time_point::min(); \
    auto _rclcpp_compat_now = std::chrono::steady_clock::now();                     \
    if (                                                                            \
      std::chrono::duration_cast<std::chrono::milliseconds>(                        \
        _rclcpp_compat_now - _rclcpp_compat_last)                                   \
        .count() >= (period_ms)) {                                                  \
      _rclcpp_compat_last = _rclcpp_compat_now;                                     \
      RCLCPP_INFO(logger, __VA_ARGS__);                                             \
    }                                                                               \
  } while (0)

#define RCLCPP_WARN_THROTTLE(logger, clock, period_ms, ...)                         \
  do {                                                                              \
    static auto _rclcpp_compat_last = std::chrono::steady_clock::time_point::min(); \
    auto _rclcpp_compat_now = std::chrono::steady_clock::now();                     \
    if (                                                                            \
      std::chrono::duration_cast<std::chrono::milliseconds>(                        \
        _rclcpp_compat_now - _rclcpp_compat_last)                                   \
        .count() >= (period_ms)) {                                                  \
      _rclcpp_compat_last = _rclcpp_compat_now;                                     \
      RCLCPP_WARN(logger, __VA_ARGS__);                                             \
    }                                                                               \
  } while (0)

#define RCLCPP_ERROR_THROTTLE(logger, clock, period_ms, ...)                        \
  do {                                                                              \
    static auto _rclcpp_compat_last = std::chrono::steady_clock::time_point::min(); \
    auto _rclcpp_compat_now = std::chrono::steady_clock::now();                     \
    if (                                                                            \
      std::chrono::duration_cast<std::chrono::milliseconds>(                        \
        _rclcpp_compat_now - _rclcpp_compat_last)                                   \
        .count() >= (period_ms)) {                                                  \
      _rclcpp_compat_last = _rclcpp_compat_now;                                     \
      RCLCPP_ERROR(logger, __VA_ARGS__);                                            \
    }                                                                               \
  } while (0)

#endif  // RCLCPP__LOGGING_HPP_
