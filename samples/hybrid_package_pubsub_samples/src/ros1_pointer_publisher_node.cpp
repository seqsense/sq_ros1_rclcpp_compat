#include "ros/ros.h"
#include "hybrid_package_msgs/StampedPointer.h"

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
