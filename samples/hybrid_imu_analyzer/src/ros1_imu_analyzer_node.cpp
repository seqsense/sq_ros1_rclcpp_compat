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

#include "hybrid_imu_analyzer/imu_analyzer.hpp"
#include "ros/ros.h"
#include "sensor_msgs/Imu.h"
#include "sq_ros1_compat/logger.hpp"
#include "sq_ros1_compat/msg_ptr.hpp"

int main(int argc, char ** argv)
{
  ros::init(argc, argv, "imu_analyzer");
  ros::NodeHandle nh;

  hybrid_imu_analyzer::ImuAnalyzer analyzer(sq_ros1_compat::get_logger(ros::this_node::getName()));

  auto callback = [&analyzer](const sensor_msgs::Imu::ConstPtr & msg) {
    analyzer.analyze(sq_ros1_compat::to_std(msg));
  };

  ros::Subscriber sub = nh.subscribe<sensor_msgs::Imu>("imu", 10, callback);
  ROS_INFO("ImuAnalyzerNode started");

  ros::spin();
  return 0;
}
