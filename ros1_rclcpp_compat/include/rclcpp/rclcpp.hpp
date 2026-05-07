#ifndef ROS1_COMPAT__RCLCPP__RCLCPP_HPP_
#define ROS1_COMPAT__RCLCPP__RCLCPP_HPP_

// ROS 1 shim for rclcpp
// Provides: rclcpp::Logger, rclcpp::Clock, RCLCPP_INFO/WARN/ERROR
// Backend: spdlog (logging), ros::Time (clock)
//
// In ROS 1 builds, this file is found via include path priority.
// In ROS 2 builds, the real rclcpp/rclcpp.hpp is used instead.

#include "rclcpp/logger.hpp"
#include "rclcpp/logging.hpp"
#include "rclcpp/clock.hpp"
#include "rclcpp/node.hpp"

#endif  // ROS1_COMPAT__RCLCPP__RCLCPP_HPP_
