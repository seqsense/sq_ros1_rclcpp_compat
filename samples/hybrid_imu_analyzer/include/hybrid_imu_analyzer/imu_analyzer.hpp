// Copyright 2026 SEQSENSE, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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

  Rpy analyze(const std::shared_ptr<const sensor_msgs::msg::Imu> & msg);

private:
  rclcpp::Logger logger_;
};

}  // namespace hybrid_imu_analyzer

#endif  // HYBRID_IMU_ANALYZER__IMU_ANALYZER_HPP_
