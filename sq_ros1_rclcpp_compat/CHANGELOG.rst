^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package sq_ros1_rclcpp_compat
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.1.0 (2026-10-28)
------------------
* Add Alpine ROS 1 release workflows (`#12 <https://github.com/seqsense/sq_ros1_rclcpp_compat/issues/12>`_)
* Contributors: Naotaka Hatao

0.0.1 (2026-08-13)
------------------
* rclcpp compat: reject Time(int, clock_type) at ROS 1 build time (`#11 <https://github.com/seqsense/sq_ros1_rclcpp_compat/issues/11>`_)
* Ship the generated compat headers without pushing the interface packages onto consumers (`#8 <https://github.com/seqsense/sq_ros1_rclcpp_compat/issues/8>`_)
* Declare the ROS 1 catkin build_type in package.xml (`#7 <https://github.com/seqsense/sq_ros1_rclcpp_compat/issues/7>`_)
* Add preprocess_ros1_interfaces and adopt ROS2-canonical msg generation (`#5 <https://github.com/seqsense/sq_ros1_rclcpp_compat/issues/5>`_)
* Apply ament_lint_auto with ament_cmake_clang_format (`#3 <https://github.com/seqsense/sq_ros1_rclcpp_compat/issues/3>`_)
* Fix build settings for ros-o
* sq_ros1_rclcpp_compat: set maintainer to Naotaka Hatao
* Switch msg compat headers to typedef + add sq_ros1_compat/ boundary helpers
* Rename package and subdirectory ros1_rclcpp_compat -> sq_ros1_rclcpp_compat
* Contributors: Naotaka Hatao
