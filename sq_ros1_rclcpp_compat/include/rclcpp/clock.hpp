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

#ifndef ROS1_COMPAT__RCLCPP__CLOCK_HPP_
#define ROS1_COMPAT__RCLCPP__CLOCK_HPP_

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>
#include <memory>

#include "ros/ros.h"

typedef enum rcl_clock_type_t
{
  RCL_CLOCK_UNINITIALIZED = 0,
  RCL_ROS_TIME = 1,
  RCL_SYSTEM_TIME = 2,
  RCL_STEADY_TIME = 3
} rcl_clock_type_t;

namespace rclcpp
{

class Duration
{
public:
  Duration(int32_t seconds, uint32_t nanoseconds)
  : nsec_(static_cast<int64_t>(seconds) * 1000000000LL + nanoseconds)
  {
  }

  explicit Duration(int64_t nanoseconds) : nsec_(nanoseconds) {}

  template <typename Rep, typename Period>
  Duration(const std::chrono::duration<Rep, Period> & d)  // NOLINT(runtime/explicit)
  : nsec_(std::chrono::duration_cast<std::chrono::nanoseconds>(d).count())
  {
  }

  Duration(const ros::Duration & d)  // NOLINT(runtime/explicit)
  : nsec_(static_cast<int64_t>(d.sec) * 1000000000LL + d.nsec)
  {
  }

  static Duration from_seconds(double seconds)
  {
    return Duration(std::chrono::duration<double>(seconds));
  }

  double seconds() const { return static_cast<double>(nsec_) / 1e9; }
  int64_t nanoseconds() const { return nsec_; }

  Duration operator+(const Duration & rhs) const { return Duration(nsec_ + rhs.nsec_); }
  Duration operator-(const Duration & rhs) const { return Duration(nsec_ - rhs.nsec_); }
  Duration operator-() const { return Duration(-nsec_); }
  Duration operator*(double scale) const { return Duration(static_cast<int64_t>(nsec_ * scale)); }

  bool operator==(const Duration & rhs) const { return nsec_ == rhs.nsec_; }
  bool operator!=(const Duration & rhs) const { return nsec_ != rhs.nsec_; }
  bool operator<(const Duration & rhs) const { return nsec_ < rhs.nsec_; }
  bool operator<=(const Duration & rhs) const { return nsec_ <= rhs.nsec_; }
  bool operator>(const Duration & rhs) const { return nsec_ > rhs.nsec_; }
  bool operator>=(const Duration & rhs) const { return nsec_ >= rhs.nsec_; }

  template <typename DurationT = std::chrono::nanoseconds>
  DurationT to_chrono() const
  {
    return std::chrono::duration_cast<DurationT>(std::chrono::nanoseconds(nsec_));
  }

  operator ros::Duration() const  // NOLINT(runtime/explicit)
  {
    return ros::Duration(
      static_cast<int32_t>(nsec_ / 1000000000LL), static_cast<int32_t>(nsec_ % 1000000000LL));
  }

private:
  int64_t nsec_;
};

class Time
{
public:
  // DO NOT ADD A DEFAULT VALUE FOR clock_type. ROS 2's rclcpp defaults to
  // RCL_SYSTEM_TIME while ROS 1 hybrid code almost always wants RCL_ROS_TIME,
  // and subtracting two Times with mismatched clock types throws at runtime.
  // Forcing callers to specify clock_type here makes that mismatch a compile
  // error instead of a latent runtime bug; matching the ROS 2 default would
  // silently silence the check. Keep the signature frozen.
  Time(int64_t nanoseconds, rcl_clock_type_t clock_type)
  : nsec_(nanoseconds), clock_type_(clock_type)
  {
  }

  // DO NOT ADD A DEFAULT VALUE FOR clock_type. Same reason as above --
  // mismatched clock types are a nasty class of bug that only surfaces at
  // Time subtraction. Keep the signature frozen.
  Time(int32_t seconds, uint32_t nanoseconds, rcl_clock_type_t clock_type)
  : nsec_(static_cast<int64_t>(seconds) * 1000000000LL + nanoseconds), clock_type_(clock_type)
  {
  }

  Time(
    const ros::Time & t,
    rcl_clock_type_t clock_type = RCL_ROS_TIME)  // NOLINT(runtime/explicit)
  : nsec_(static_cast<int64_t>(t.sec) * 1000000000LL + t.nsec), clock_type_(clock_type)
  {
  }

  double seconds() const { return static_cast<double>(nsec_) / 1e9; }
  int64_t nanoseconds() const { return nsec_; }
  rcl_clock_type_t get_clock_type() const { return clock_type_; }

  operator ros::Time() const  // NOLINT(runtime/explicit)
  {
    return ros::Time(
      static_cast<uint32_t>(nsec_ / 1000000000LL), static_cast<uint32_t>(nsec_ % 1000000000LL));
  }

  static Time max(rcl_clock_type_t clock_type = RCL_SYSTEM_TIME)
  {
    return Time(std::numeric_limits<int64_t>::max(), clock_type);
  }

  Duration operator-(const Time & rhs) const { return Duration(nsec_ - rhs.nsec_); }

  Time operator+(const Duration & d) const { return Time(nsec_ + d.nanoseconds(), clock_type_); }
  Time operator-(const Duration & d) const { return Time(nsec_ - d.nanoseconds(), clock_type_); }
  Time & operator+=(const Duration & d)
  {
    nsec_ += d.nanoseconds();
    return *this;
  }
  Time & operator-=(const Duration & d)
  {
    nsec_ -= d.nanoseconds();
    return *this;
  }

  bool operator==(const Time & rhs) const { return nsec_ == rhs.nsec_; }
  bool operator!=(const Time & rhs) const { return nsec_ != rhs.nsec_; }
  bool operator<(const Time & rhs) const { return nsec_ < rhs.nsec_; }
  bool operator<=(const Time & rhs) const { return nsec_ <= rhs.nsec_; }
  bool operator>(const Time & rhs) const { return nsec_ > rhs.nsec_; }
  bool operator>=(const Time & rhs) const { return nsec_ >= rhs.nsec_; }

private:
  int64_t nsec_;
  rcl_clock_type_t clock_type_;
};

class Clock
{
public:
  using SharedPtr = std::shared_ptr<Clock>;
  using ConstSharedPtr = std::shared_ptr<const Clock>;

  explicit Clock(rcl_clock_type_t type = RCL_ROS_TIME) : type_(type) {}

  Time now() const
  {
    if (type_ == RCL_ROS_TIME) {
      return Time(ros::Time::now(), RCL_ROS_TIME);
    }
    // RCL_SYSTEM_TIME / RCL_STEADY_TIME -> ros::WallTime
    const ros::WallTime wt = ros::WallTime::now();
    return Time(static_cast<int64_t>(wt.sec) * 1000000000LL + wt.nsec, type_);
  }

  rcl_clock_type_t get_clock_type() const { return type_; }

private:
  rcl_clock_type_t type_;
};

}  // namespace rclcpp

#endif  // ROS1_COMPAT__RCLCPP__CLOCK_HPP_
