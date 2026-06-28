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

#ifndef ROS1_COMPAT__RCLCPP__LOGGING_HPP_
#define ROS1_COMPAT__RCLCPP__LOGGING_HPP_

#include <spdlog/spdlog.h>

#include <cstdio>

#include "rclcpp/logger.hpp"

#define RCLCPP_DEBUG(logger, ...)                                               \
  do {                                                                          \
    char _rclcpp_compat_buf[2048];                                              \
    std::snprintf(_rclcpp_compat_buf, sizeof(_rclcpp_compat_buf), __VA_ARGS__); \
    (logger).get_spdlog_logger()->debug(_rclcpp_compat_buf);                    \
  } while (0)

#define RCLCPP_INFO(logger, ...)                                                \
  do {                                                                          \
    char _rclcpp_compat_buf[2048];                                              \
    std::snprintf(_rclcpp_compat_buf, sizeof(_rclcpp_compat_buf), __VA_ARGS__); \
    (logger).get_spdlog_logger()->info(_rclcpp_compat_buf);                     \
  } while (0)

#define RCLCPP_WARN(logger, ...)                                                \
  do {                                                                          \
    char _rclcpp_compat_buf[2048];                                              \
    std::snprintf(_rclcpp_compat_buf, sizeof(_rclcpp_compat_buf), __VA_ARGS__); \
    (logger).get_spdlog_logger()->warn(_rclcpp_compat_buf);                     \
  } while (0)

#define RCLCPP_ERROR(logger, ...)                                               \
  do {                                                                          \
    char _rclcpp_compat_buf[2048];                                              \
    std::snprintf(_rclcpp_compat_buf, sizeof(_rclcpp_compat_buf), __VA_ARGS__); \
    (logger).get_spdlog_logger()->error(_rclcpp_compat_buf);                    \
  } while (0)

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

#endif  // ROS1_COMPAT__RCLCPP__LOGGING_HPP_
