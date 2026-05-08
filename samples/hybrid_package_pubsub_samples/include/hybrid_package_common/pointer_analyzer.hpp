#ifndef HYBRID_PACKAGE_COMMON__POINTER_ANALYZER_HPP_
#define HYBRID_PACKAGE_COMMON__POINTER_ANALYZER_HPP_

#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "hybrid_package_msgs/msg/stamped_pointer.hpp"

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
