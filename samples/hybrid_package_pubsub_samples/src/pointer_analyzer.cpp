#include "hybrid_package_common/pointer_analyzer.hpp"

namespace hybrid_package_common
{

PointerAnalyzer::PointerAnalyzer(const rclcpp::Logger & logger)
: logger_(logger) {}

bool PointerAnalyzer::analyze(
  const std::shared_ptr<const hybrid_package_msgs::msg::StampedPointer> & msg)
{
  const double time_diff_sec =
    (rclcpp::Clock(RCL_ROS_TIME).now() - msg->header.stamp).seconds();

  const uint64_t received_ptr = reinterpret_cast<uint64_t>(msg.get());
  const bool is_zero_copy = (msg->pointer_value == received_ptr);

  RCLCPP_INFO(
    logger_, "stored=0x%lx received=0x%lx zero_copy=%s time_diff=%.6fs",
    msg->pointer_value, received_ptr, is_zero_copy ? "true" : "false", time_diff_sec);

  return is_zero_copy;
}

}  // namespace hybrid_package_common
