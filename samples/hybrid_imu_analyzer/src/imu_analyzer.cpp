#include "hybrid_imu_analyzer/imu_analyzer.hpp"

#include <cmath>

namespace hybrid_imu_analyzer
{

ImuAnalyzer::ImuAnalyzer(const rclcpp::Logger & logger)
: logger_(logger) {}

Rpy ImuAnalyzer::analyze(const sensor_msgs::msg::Imu::ConstSharedPtr & msg)
{
  // Quaternion to RPY (ZYX Euler angles)
  const double qx = msg->orientation.x;
  const double qy = msg->orientation.y;
  const double qz = msg->orientation.z;
  const double qw = msg->orientation.w;

  Rpy rpy;
  // roll (x-axis rotation)
  const double sinr_cosp = 2.0 * (qw * qx + qy * qz);
  const double cosr_cosp = 1.0 - 2.0 * (qx * qx + qy * qy);
  rpy.roll = std::atan2(sinr_cosp, cosr_cosp);

  // pitch (y-axis rotation)
  const double sinp = 2.0 * (qw * qy - qz * qx);
  rpy.pitch = (std::fabs(sinp) >= 1.0) ?
    std::copysign(M_PI / 2.0, sinp) :
    std::asin(sinp);

  // yaw (z-axis rotation)
  const double siny_cosp = 2.0 * (qw * qz + qx * qy);
  const double cosy_cosp = 1.0 - 2.0 * (qy * qy + qz * qz);
  rpy.yaw = std::atan2(siny_cosp, cosy_cosp);

  // Compute message latency
  const rclcpp::Time stamp(msg->header.stamp);
  const double latency = (rclcpp::Clock(RCL_ROS_TIME).now() - stamp).seconds();

  RCLCPP_INFO(
    logger_, "RPY=[%.2f, %.2f, %.2f] deg  latency=%.6fs",
    rpy.roll * 180.0 / M_PI,
    rpy.pitch * 180.0 / M_PI,
    rpy.yaw * 180.0 / M_PI,
    latency);

  return rpy;
}

}  // namespace hybrid_imu_analyzer
