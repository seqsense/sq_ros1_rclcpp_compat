#ifndef SQ_ROS1_COMPAT__LOGGER_HPP_
#define SQ_ROS1_COMPAT__LOGGER_HPP_

// Logger factory for ROS 1 interface code.
//
// Hybridized logic classes accept rclcpp::Logger at construction so they
// can emit log messages without depending on rclcpp::Node / ros::NodeHandle.
// The ROS 1 IF layer needs to construct one such logger to pass in.
//
// This wrapper exists so the IF code does not have to include
// rclcpp/rclcpp.hpp directly: the convention for hybridized packages is
// that ROS 1 IF code only reaches into compat through sq_ros1_compat/*.
// The returned object is rclcpp::Logger (defined by the rclcpp shim on
// ROS 1, by the real rclcpp on ROS 2); the IF code typically forwards it
// straight into the logic class constructor without naming the type.

#include <string>

#include "rclcpp/logger.hpp"

namespace sq_ros1_compat
{

inline rclcpp::Logger
get_logger(const std::string & name)
{
  return rclcpp::get_logger(name);
}

}  // namespace sq_ros1_compat

#endif  // SQ_ROS1_COMPAT__LOGGER_HPP_
