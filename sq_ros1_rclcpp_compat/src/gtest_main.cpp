// gtest_main equivalent for ROS 1 unit tests using sq_ros1_rclcpp_compat.
//
// catkin_add_gtest does not link gtest's own gtest_main, so without this
// library every test would have to define its own main(). This main()
// also calls ros::Time::init() once before RUN_ALL_TESTS, so tests that
// touch rclcpp::Clock(RCL_ROS_TIME).now() (which the compat shim routes
// through ros::Time::now()) work without per-test boilerplate.
//
// Tests link this with:
//
//   target_link_libraries(<test> <logic_lib> sq_ros1_rclcpp_compat_gtest_main
//                         ${catkin_LIBRARIES})
//
// On ROS 2, ament_add_gtest already supplies gtest_main and rclcpp does
// not need to be initialised for clock/logger use, so this library is not
// needed there.

#include <gtest/gtest.h>
#include <ros/time.h>

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  ros::Time::init();
  return RUN_ALL_TESTS();
}
