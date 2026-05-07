// Pure ROS 1 style node — no compat headers, no rclcpp shim.
// Uses the common ImuAnalyzer library via its header (which internally uses compat).

#include "ros/ros.h"
#include "sensor_msgs/Imu.h"
#include "hybrid_imu_analyzer/imu_analyzer.hpp"

int main(int argc, char ** argv)
{
  ros::init(argc, argv, "imu_analyzer");
  ros::NodeHandle nh;

  hybrid_imu_analyzer::ImuAnalyzer analyzer(
    rclcpp::get_logger(ros::this_node::getName()));

  auto callback = [&analyzer](const sensor_msgs::Imu::ConstPtr & msg) {
      analyzer.analyze(msg);
    };

  ros::Subscriber sub = nh.subscribe<sensor_msgs::Imu>("imu", 10, callback);
  ROS_INFO("ImuAnalyzerNode started");

  ros::spin();
  return 0;
}
