#ifndef HYBRID_IMU_ANALYZER__IMU_ANALYZER_HPP_
#define HYBRID_IMU_ANALYZER__IMU_ANALYZER_HPP_

#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"

namespace hybrid_imu_analyzer
{

struct Rpy
{
  double roll;
  double pitch;
  double yaw;
};

class ImuAnalyzer
{
public:
  explicit ImuAnalyzer(const rclcpp::Logger & logger);

  /// Analyze IMU message: compute RPY from quaternion, log latency.
  /// Returns the computed roll/pitch/yaw.
  Rpy analyze(const std::shared_ptr<const sensor_msgs::msg::Imu> & msg);

private:
  rclcpp::Logger logger_;
};

}  // namespace hybrid_imu_analyzer

#endif  // HYBRID_IMU_ANALYZER__IMU_ANALYZER_HPP_
