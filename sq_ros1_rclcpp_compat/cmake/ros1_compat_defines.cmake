# Automatically define IS_ROS1_BUILD for all packages that depend on sq_ros1_rclcpp_compat.
# This allows shared headers to use #ifdef IS_ROS1_BUILD for ROS1-specific APIs
# (XmlRpc, dynamic_reconfigure, etc.) that have no compat shim equivalent.
add_compile_definitions(IS_ROS1_BUILD)

# Ubuntu 24.04+: spdlog uses external fmt (SPDLOG_FMT_EXTERNAL=1), so any target
# linking ${catkin_LIBRARIES} needs fmt::fmt. catkin_package(DEPENDS fmt) propagates
# fmt::fmt via catkin_LIBRARIES, but the imported target must be created first in
# each downstream build context.
find_package(fmt QUIET)
