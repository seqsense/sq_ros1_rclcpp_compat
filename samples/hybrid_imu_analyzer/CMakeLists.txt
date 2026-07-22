cmake_minimum_required(VERSION 3.5)
project(hybrid_imu_analyzer)

set(CMAKE_CXX_STANDARD 17)

if($ENV{ROS_VERSION} EQUAL 1)
  message(STATUS "Building for ROS 1 (Noetic)")

  find_package(catkin REQUIRED COMPONENTS roscpp sensor_msgs sq_ros1_rclcpp_compat)
  catkin_package()
  include_directories(include ${catkin_INCLUDE_DIRS})

  # Common algorithm library. Named after the package so it does not clash
  # with another package's libimu_analyzer.so in the shared catkin devel
  # lib dir.
  add_library(hybrid_imu_analyzer src/imu_analyzer.cpp)
  target_include_directories(hybrid_imu_analyzer PUBLIC include)
  target_link_libraries(hybrid_imu_analyzer ${catkin_LIBRARIES})
  add_dependencies(hybrid_imu_analyzer ${catkin_EXPORTED_TARGETS})

  # ROS 1 node
  add_executable(imu_analyzer_node src/ros1_imu_analyzer_node.cpp)
  target_link_libraries(imu_analyzer_node hybrid_imu_analyzer ${catkin_LIBRARIES})
  add_dependencies(imu_analyzer_node ${catkin_EXPORTED_TARGETS})

  install(TARGETS imu_analyzer_node
          RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})

  if(CATKIN_ENABLE_TESTING)
    catkin_add_gtest(test_imu_analyzer test/src/test_imu_analyzer.cpp)
    # main() (with ros::Time::init) comes from sq_ros1_rclcpp_compat_gtest_main,
    # which sq_ros1_rclcpp_compat exports through ${catkin_LIBRARIES} when its
    # CATKIN_ENABLE_TESTING is on. catkin_add_gtest does not link gtest_main
    # on its own, so without this we would have to define main() per test.
    target_link_libraries(test_imu_analyzer hybrid_imu_analyzer ${catkin_LIBRARIES})
    add_dependencies(test_imu_analyzer ${catkin_EXPORTED_TARGETS})
  endif()

elseif($ENV{ROS_VERSION} EQUAL 2)
  message(STATUS "Building for ROS 2 (Jazzy)")

  find_package(ament_cmake_auto REQUIRED)
  ament_auto_find_build_dependencies()

  # Common algorithm library
  ament_auto_add_library(hybrid_imu_analyzer SHARED src/imu_analyzer.cpp)

  # ROS 2 component node
  ament_auto_add_library(imu_analyzer_node SHARED
    src/ros2_imu_analyzer_node.cpp)
  target_link_libraries(imu_analyzer_node hybrid_imu_analyzer)
  rclcpp_components_register_node(
    imu_analyzer_node PLUGIN "hybrid_imu_analyzer::ImuAnalyzerNode" EXECUTABLE
    imu_analyzer_node_exec)

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
    ament_add_gtest(test_imu_analyzer test/src/test_imu_analyzer.cpp)
    target_link_libraries(test_imu_analyzer hybrid_imu_analyzer)
    ament_target_dependencies(test_imu_analyzer rclcpp sensor_msgs)
  endif()

  ament_auto_package()

else()
  message(
    FATAL_ERROR "ROS_VERSION environment variable not set or unsupported.")
endif()
