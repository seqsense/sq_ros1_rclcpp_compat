#ifndef SQ_ROS1_COMPAT__MSG_PTR_HPP_
#define SQ_ROS1_COMPAT__MSG_PTR_HPP_

// Boost <-> std shared_ptr conversion helpers used at the ROS 1 interface
// boundary. Both directions use the aliasing-constructor trick so the
// returned shared_ptr keeps the source pointer alive without copying the
// underlying message; only the control-block refcount is bumped.
//
// This header intentionally lives outside the rclcpp/ shim tree: it
// contains no rclcpp surface (no Logger, Clock, Time, logging macros,
// nor the ROS 2 message namespace aliases) and is meant to be included
// from ROS 1 native code. The convention for hybridized packages is:
//
//   - Logic layer (.cpp/.hpp shared by both builds): uses rclcpp/<...>
//     and pkg/msg/<type>.hpp; declares shared_ptr arguments as
//     std::shared_ptr<const T>.
//   - ROS 1 interface (ros1_*.cpp): stays ROS 1 native (ros/, native
//     msg headers, your logic header); may additionally include
//     sq_ros1_compat/msg_ptr.hpp here to bridge boost::shared_ptr
//     subscriber callbacks to the logic layer's std::shared_ptr.
//     (rclcpp/rclcpp.hpp is allowed only for the rclcpp::get_logger()
//     glue needed to construct the logic class.)
//   - ROS 2 interface (ros2_*.cpp): plain rclcpp; no conversion needed
//     because subscriber callbacks already deliver std::shared_ptr.

#include <memory>

#include <boost/shared_ptr.hpp>

namespace sq_ros1_compat
{

template<typename T>
inline std::shared_ptr<const T>
to_std(const boost::shared_ptr<const T> & b)
{
  return std::shared_ptr<const T>(b.get(), [b](const T *) {});
}

template<typename T>
inline std::shared_ptr<T>
to_std(const boost::shared_ptr<T> & b)
{
  return std::shared_ptr<T>(b.get(), [b](T *) {});
}

template<typename T>
inline boost::shared_ptr<T>
to_boost(const std::shared_ptr<T> & s)
{
  return boost::shared_ptr<T>(s, s.get());
}

template<typename T>
inline boost::shared_ptr<const T>
to_boost(const std::shared_ptr<const T> & s)
{
  return boost::shared_ptr<const T>(s, s.get());
}

}  // namespace sq_ros1_compat

#endif  // SQ_ROS1_COMPAT__MSG_PTR_HPP_
