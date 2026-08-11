#ifndef DIAGNOSTIC_MSGS__SRV__SELFTEST_HPP_
#define DIAGNOSTIC_MSGS__SRV__SELFTEST_HPP_

// Auto-generated ROS 2-style compatibility header for ROS 1.
// Provides: diagnostic_msgs::srv::SelfTest as a using-alias for the ROS 1
// type diagnostic_msgs::SelfTest. Logic-layer code that declares shared_ptr
// arguments as std::shared_ptr<const T> works on both builds; on ROS 1
// the IF layer converts boost::shared_ptr -> std::shared_ptr via
// sq_ros1_compat::to_std() (no deep copy).

#include "diagnostic_msgs/SelfTest.h"

namespace diagnostic_msgs {
namespace srv {
using SelfTest = ::diagnostic_msgs::SelfTest;
}  // namespace srv
}  // namespace diagnostic_msgs

#endif  // DIAGNOSTIC_MSGS__SRV__SELFTEST_HPP_
