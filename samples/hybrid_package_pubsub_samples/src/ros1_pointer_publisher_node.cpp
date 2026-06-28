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

#include "hybrid_package_msgs/StampedPointer.h"
#include "ros/ros.h"

int main(int argc, char ** argv)
{
  ros::init(argc, argv, "pointer_publisher");
  ros::NodeHandle nh;

  ros::Publisher pub = nh.advertise<hybrid_package_msgs::StampedPointer>("stamped_pointer", 10);
  ros::Rate rate(1.0);

  ROS_INFO("PointerPublisherNode started");

  while (ros::ok()) {
    hybrid_package_msgs::StampedPointer msg;
    msg.header.stamp = ros::Time::now();
    msg.pointer_value = reinterpret_cast<uint64_t>(&msg);
    ROS_INFO("Publishing pointer: 0x%lx", msg.pointer_value);
    pub.publish(msg);

    ros::spinOnce();
    rate.sleep();
  }

  return 0;
}
