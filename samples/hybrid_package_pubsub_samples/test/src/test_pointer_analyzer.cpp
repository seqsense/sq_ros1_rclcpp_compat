// Unit tests for hybrid_package_common::PointerAnalyzer.
//
// PointerAnalyzer::analyze(msg) compares msg->pointer_value (stored by the
// publisher before move-publish) with reinterpret_cast<uint64_t>(msg.get())
// (the pointer the subscriber actually received). When intra-process zero-
// copy is in effect the two addresses match; with copy-mode delivery they
// don't. These tests poke pointer_value directly to exercise both paths
// without bringing up a publisher.
//
// On now()
// --------
// analyze() also computes a wall-clock time_diff via
// rclcpp::Clock(RCL_ROS_TIME).now() - msg->header.stamp. We stamp the msg
// with the same clock just before calling analyze() so the subtraction is
// well defined; the time_diff value itself is logged but not asserted.
//
// On main()
// ---------
// ROS 2: ament_add_gtest auto-links gtest_main, no main() needed here.
// ROS 1: the test links sq_ros1_rclcpp_compat_gtest_main (exported by
// sq_ros1_rclcpp_compat via ${catkin_LIBRARIES}), which supplies a main()
// that calls ros::Time::init() before RUN_ALL_TESTS. The test source is
// therefore free of build-specific preprocessor branches.

#include "hybrid_package_common/pointer_analyzer.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>

#include "hybrid_package_msgs/msg/stamped_pointer.hpp"
#include "rclcpp/rclcpp.hpp"

namespace
{

// The hybrid logic class accepts std::shared_ptr<const T>, so the test
// path uses std::shared_ptr on both builds. ROS 1 callbacks deliver
// boost::shared_ptr at run time, but the IF layer converts via
// sq_ros1_compat::to_std() before reaching analyze() — none of that
// surfaces in this gtest, which calls analyze() directly.
std::shared_ptr<hybrid_package_msgs::msg::StampedPointer> make_msg()
{
  auto msg = std::make_shared<hybrid_package_msgs::msg::StampedPointer>();
  msg->header.stamp = rclcpp::Clock(RCL_ROS_TIME).now();
  return msg;
}

}  // namespace

class PointerAnalyzerTest : public ::testing::Test
{
protected:
  hybrid_package_common::PointerAnalyzer analyzer_{
    rclcpp::get_logger("pointer_analyzer_test")};
};

TEST_F(PointerAnalyzerTest, MatchingPointerIsReportedAsZeroCopy)
{
  auto msg = make_msg();
  msg->pointer_value = reinterpret_cast<uint64_t>(msg.get());

  EXPECT_TRUE(analyzer_.analyze(msg));
}

TEST_F(PointerAnalyzerTest, MismatchingPointerIsReportedAsCopy)
{
  auto msg = make_msg();
  // Any value that cannot match msg.get() works; pick a recognisable one.
  msg->pointer_value = 0xDEADBEEFu;

  EXPECT_FALSE(analyzer_.analyze(msg));
}

TEST_F(PointerAnalyzerTest, DefaultZeroPointerValueIsReportedAsCopy)
{
  // Fresh message: pointer_value defaults to 0, msg.get() is non-zero,
  // so the two cannot match.
  auto msg = make_msg();
  ASSERT_EQ(msg->pointer_value, 0u);

  EXPECT_FALSE(analyzer_.analyze(msg));
}
