// ROS 2 gtest equivalent of ros1_test_copy.cpp
// Subscribes to stamped_pointer and verifies that messages are copied
// (i.e. pointer_value != address of received message) in inter-process communication.

#include <gtest/gtest.h>

#include <atomic>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "hybrid_package_msgs/msg/stamped_pointer.hpp"

class CopyTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    node_ = std::make_shared<rclcpp::Node>("test_copy");
    copy_confirmed_.store(false);
    sub_ = node_->create_subscription<hybrid_package_msgs::msg::StampedPointer>(
      "stamped_pointer", 10,
      [this](const hybrid_package_msgs::msg::StampedPointer::ConstSharedPtr & msg) {
        const uint64_t received_ptr = reinterpret_cast<uint64_t>(msg.get());
        if (msg->pointer_value != received_ptr) {
          copy_confirmed_.store(true);
        }
      });
  }

  void TearDown() override
  {
    sub_.reset();
    node_.reset();
  }

  rclcpp::Node::SharedPtr node_;
  rclcpp::Subscription<hybrid_package_msgs::msg::StampedPointer>::SharedPtr sub_;
  std::atomic<bool> copy_confirmed_;
};

TEST_F(CopyTest, MessageIsCopied)
{
  const auto deadline = node_->now() + rclcpp::Duration(20, 0);
  rclcpp::WallRate rate(10.0);
  while (rclcpp::ok() && !copy_confirmed_.load() && node_->now() < deadline) {
    rclcpp::spin_some(node_);
    rate.sleep();
  }
  EXPECT_TRUE(copy_confirmed_.load()) << "Message should be copied in inter-process communication";
}

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  testing::InitGoogleTest(&argc, argv);
  const int result = RUN_ALL_TESTS();
  rclcpp::shutdown();
  return result;
}
