#include "ros/ros.h"
#include "hybrid_package_msgs/StampedPointer.h"
#include "sq_ros1_compat/logger.hpp"
#include "sq_ros1_compat/msg_ptr.hpp"

#include "hybrid_package_common/pointer_analyzer.hpp"

int main(int argc, char ** argv)
{
  ros::init(argc, argv, "pointer_subscriber_copy");
  ros::NodeHandle nh;

  hybrid_package_common::PointerAnalyzer analyzer(
    sq_ros1_compat::get_logger(ros::this_node::getName()));

  auto callback = [&analyzer](const hybrid_package_msgs::StampedPointer::ConstPtr & msg) {
      const bool is_zero_copy = analyzer.analyze(sq_ros1_compat::to_std(msg));
      ROS_INFO("%s", is_zero_copy ? "RESULT:UNEXPECTED_ZERO_COPY" : "RESULT:COPY_OK");
    };

  ros::Subscriber sub = nh.subscribe<hybrid_package_msgs::StampedPointer>(
    "stamped_pointer", 10, callback);
  ROS_INFO("PointerSubscriberCopy started");

  ros::spin();
  return 0;
}
