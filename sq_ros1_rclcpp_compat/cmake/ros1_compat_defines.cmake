# Automatically define IS_ROS1_BUILD for all packages that depend on sq_ros1_rclcpp_compat.
# This allows shared headers to use #ifdef IS_ROS1_BUILD for ROS1-specific APIs
# (XmlRpc, dynamic_reconfigure, etc.) that have no compat shim equivalent.
add_compile_definitions(IS_ROS1_BUILD)
