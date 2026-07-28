cmake_minimum_required(VERSION 3.5)
project(hybrid_package_msgs)

if($ENV{ROS_VERSION} EQUAL 1)
  message(STATUS "Building for ROS 1")

  find_package(catkin REQUIRED COMPONENTS std_msgs message_generation
                                          sq_ros1_rclcpp_compat)
  add_message_files(FILES StampedPointer.msg)
  generate_messages(DEPENDENCIES std_msgs)
  catkin_package(CATKIN_DEPENDS std_msgs message_runtime)

  # Generate ROS 2-style compatibility headers (function from
  # sq_ros1_rclcpp_compat)
  generate_ros1_compat_headers(
    PACKAGE "${PROJECT_NAME}" MSG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/msg"
    OUTPUT_DIR "${CATKIN_DEVEL_PREFIX}/include")

elseif($ENV{ROS_VERSION} EQUAL 2)
  message(STATUS "Building for ROS 2")

  find_package(ament_cmake_auto REQUIRED)
  ament_auto_find_build_dependencies()

  # Auto-discover msg/srv/action files and derive DEPENDENCIES from
  # package.xml (no need to list interface files explicitly).
  rosidl_auto_generate_interfaces()

  if(BUILD_TESTING)
    find_package(ament_lint_auto REQUIRED)
    list(APPEND AMENT_LINT_AUTO_EXCLUDE ament_cmake_uncrustify)
    ament_lint_auto_find_test_dependencies()
  endif()

  ament_auto_package()

else()
  message(
    FATAL_ERROR "ROS_VERSION environment variable not set or unsupported.")
endif()
