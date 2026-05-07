#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "hybrid_imu_analyzer/imu_analyzer.hpp"

namespace hybrid_imu_analyzer
{

class ImuAnalyzerNode : public rclcpp::Node
{
public:
  explicit ImuAnalyzerNode(const rclcpp::NodeOptions & options)
  : rclcpp::Node("imu_analyzer", options),
    analyzer_(this->get_logger())
  {
    sub_ = this->create_subscription<sensor_msgs::msg::Imu>(
      "imu", 10,
      std::bind(&ImuAnalyzerNode::callback, this, std::placeholders::_1));
    RCLCPP_INFO(this->get_logger(), "ImuAnalyzerNode started");
  }

private:
  void callback(const sensor_msgs::msg::Imu::ConstSharedPtr & msg)
  {
    analyzer_.analyze(msg);
  }

  ImuAnalyzer analyzer_;
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr sub_;
};

}  // namespace hybrid_imu_analyzer

#include "rclcpp_components/register_node_macro.hpp"
RCLCPP_COMPONENTS_REGISTER_NODE(hybrid_imu_analyzer::ImuAnalyzerNode)
