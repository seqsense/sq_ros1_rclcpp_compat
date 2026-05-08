// ROS 1 native interface for hybrid_imu_analyzer.
//
// The shared logic (hybrid_imu_analyzer::ImuAnalyzer) is hybridized via
// the sq_ros1_rclcpp_compat shim and accepts a std::shared_ptr<const T>
// message argument plus an rclcpp::Logger. This IF stays ROS 1 native
// (ros::NodeHandle, ROS_INFO, sensor_msgs::Imu) and only reaches into
// compat through sq_ros1_compat/* — never through rclcpp/*.

#include "ros/ros.h"
#include "sensor_msgs/Imu.h"
#include "sq_ros1_compat/logger.hpp"
#include "sq_ros1_compat/msg_ptr.hpp"

#include "hybrid_imu_analyzer/imu_analyzer.hpp"

int main(int argc, char ** argv)
{
  ros::init(argc, argv, "imu_analyzer");
  ros::NodeHandle nh;

  hybrid_imu_analyzer::ImuAnalyzer analyzer(
    sq_ros1_compat::get_logger(ros::this_node::getName()));

  auto callback = [&analyzer](const sensor_msgs::Imu::ConstPtr & msg) {
      analyzer.analyze(sq_ros1_compat::to_std(msg));
    };

  ros::Subscriber sub = nh.subscribe<sensor_msgs::Imu>("imu", 10, callback);
  ROS_INFO("ImuAnalyzerNode started");

  ros::spin();
  return 0;
}
