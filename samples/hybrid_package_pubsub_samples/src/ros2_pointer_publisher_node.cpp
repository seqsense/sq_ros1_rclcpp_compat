#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "hybrid_package_msgs/msg/stamped_pointer.hpp"

using namespace std::chrono_literals;

namespace hybrid_package_pubsub_samples
{
class PointerPublisherNode : public rclcpp::Node
{
public:
  explicit PointerPublisherNode(const rclcpp::NodeOptions & options)
  : rclcpp::Node("pointer_publisher", options)
  {
    publisher_ = this->create_publisher<hybrid_package_msgs::msg::StampedPointer>("stamped_pointer",
        10);
    timer_ = this->create_wall_timer(1s, std::bind(&PointerPublisherNode::timer_callback, this));
    RCLCPP_INFO(this->get_logger(), "PointerPublisherNode started");
  }

private:
  void timer_callback()
  {
    auto msg = std::make_unique<hybrid_package_msgs::msg::StampedPointer>();
    msg->header.stamp = this->now();
    msg->pointer_value = reinterpret_cast<uint64_t>(msg.get());
    RCLCPP_INFO(this->get_logger(), "Publishing pointer: 0x%lx", msg->pointer_value);
    publisher_->publish(std::move(msg));
  }

  rclcpp::Publisher<hybrid_package_msgs::msg::StampedPointer>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};
}  // namespace hybrid_package_pubsub_samples

#include "rclcpp_components/register_node_macro.hpp"
RCLCPP_COMPONENTS_REGISTER_NODE(hybrid_package_pubsub_samples::PointerPublisherNode)
