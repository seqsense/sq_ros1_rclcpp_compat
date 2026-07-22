cmake_minimum_required(VERSION 3.5)
project(hybrid_package_pubsub_samples)

set(CMAKE_CXX_STANDARD 17)

if($ENV{ROS_VERSION} EQUAL 1)
  message(STATUS "Building for ROS 1 (Noetic)")

  find_package(catkin REQUIRED COMPONENTS roscpp hybrid_package_msgs
                                          sq_ros1_rclcpp_compat)
  catkin_package()
  include_directories(include ${catkin_INCLUDE_DIRS})

  # Common algorithm library (uses rclcpp shim from sq_ros1_rclcpp_compat)
  add_library(hybrid_package_common src/pointer_analyzer.cpp)
  target_include_directories(hybrid_package_common PUBLIC include)
  target_link_libraries(hybrid_package_common ${catkin_LIBRARIES})
  add_dependencies(hybrid_package_common ${catkin_EXPORTED_TARGETS})

  # Publisher node
  add_executable(pointer_publisher_node src/ros1_pointer_publisher_node.cpp)
  target_link_libraries(pointer_publisher_node ${catkin_LIBRARIES})
  add_dependencies(pointer_publisher_node ${catkin_EXPORTED_TARGETS})

  # Subscriber (copy) node
  add_executable(pointer_subscriber_node src/ros1_pointer_subscriber_node.cpp)
  target_link_libraries(pointer_subscriber_node hybrid_package_common
                        ${catkin_LIBRARIES})
  add_dependencies(pointer_subscriber_node ${catkin_EXPORTED_TARGETS})

  install(TARGETS pointer_publisher_node pointer_subscriber_node
          RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})

  if(CATKIN_ENABLE_TESTING)
    find_package(rostest REQUIRED)
    add_rostest_gtest(test_copy_node test/test/test_copy.launch
                      test/src/ros1_test_copy.cpp)
    target_link_libraries(test_copy_node hybrid_package_common
                          ${catkin_LIBRARIES})
    add_dependencies(test_copy_node ${catkin_EXPORTED_TARGETS})

    # PointerAnalyzer unit test. main() (with ros::Time::init) comes from
    # sq_ros1_rclcpp_compat_gtest_main, exported via ${catkin_LIBRARIES}.
    catkin_add_gtest(test_pointer_analyzer test/src/test_pointer_analyzer.cpp)
    target_link_libraries(test_pointer_analyzer hybrid_package_common
                          ${catkin_LIBRARIES})
    add_dependencies(test_pointer_analyzer ${catkin_EXPORTED_TARGETS})
  endif()

elseif($ENV{ROS_VERSION} EQUAL 2)
  message(STATUS "Building for ROS 2 (Jazzy)")
  add_compile_definitions(IS_ROS2_BUILD)

  find_package(ament_cmake_auto REQUIRED)
  ament_auto_find_build_dependencies()

  # Common algorithm library
  ament_auto_add_library(hybrid_package_common SHARED src/pointer_analyzer.cpp)

  # Publisher component
  ament_auto_add_library(pointer_publisher_node SHARED
    src/ros2_pointer_publisher_node.cpp)
  rclcpp_components_register_node(
    pointer_publisher_node PLUGIN
    "hybrid_package_pubsub_samples::PointerPublisherNode" EXECUTABLE
    pointer_publisher_node_exec)

  # Subscriber component
  ament_auto_add_library(pointer_subscriber_node SHARED
    src/ros2_pointer_subscriber_node.cpp)
  rclcpp_components_register_node(
    pointer_subscriber_node PLUGIN
    "hybrid_package_pubsub_samples::PointerSubscriberNode" EXECUTABLE
    pointer_subscriber_node_exec)

  if(BUILD_TESTING)
    find_package(ament_lint_auto REQUIRED)
    list(APPEND AMENT_LINT_AUTO_EXCLUDE ament_cmake_uncrustify)
    # clang-format ships a different bundled .clang-format per distro (humble
    # lacks "AfterEnum: true"), so the same source cannot satisfy every distro's
    # formatter. Formatting is a style-only check, so run it on jazzy (the
    # source of truth) and skip it on humble.
    if("$ENV{ROS_DISTRO}" STREQUAL "humble")
      list(APPEND AMENT_LINT_AUTO_EXCLUDE ament_cmake_clang_format)
    endif()
    ament_lint_auto_find_test_dependencies()

    find_package(ament_cmake_gtest REQUIRED)
    find_package(launch_testing_ament_cmake REQUIRED)

    # Python-based launch tests (assert on stdout)
    add_launch_test(test/test/test_zero_copy_launch.py)
    add_launch_test(test/test/test_copy_launch.py)

    # C++ gtest via launch_testing (rostest migration pattern)
    ament_add_gtest_executable(test_copy_gtest_node test/src/ros2_test_copy.cpp)
    ament_target_dependencies(test_copy_gtest_node rclcpp hybrid_package_msgs)
    install(TARGETS test_copy_gtest_node DESTINATION lib/${PROJECT_NAME})
    add_launch_test(test/test/test_copy_gtest_launch.py)

    # PointerAnalyzer unit test
    ament_add_gtest(test_pointer_analyzer test/src/test_pointer_analyzer.cpp)
    target_link_libraries(test_pointer_analyzer hybrid_package_common)
    ament_target_dependencies(test_pointer_analyzer rclcpp hybrid_package_msgs)
  endif()

  ament_auto_package(INSTALL_TO_SHARE launch)

else()
  message(
    FATAL_ERROR "ROS_VERSION environment variable not set or unsupported.")
endif()
