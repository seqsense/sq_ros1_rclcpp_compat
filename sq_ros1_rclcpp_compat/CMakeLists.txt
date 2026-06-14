cmake_minimum_required(VERSION 3.5)
project(sq_ros1_rclcpp_compat)

if($ENV{ROS_VERSION} EQUAL 1)
  message(STATUS "Building sq_ros1_rclcpp_compat for ROS 1")

  find_package(catkin REQUIRED COMPONENTS roscpp)
  find_package(spdlog REQUIRED)
  # Ubuntu 24.04+: spdlog uses external fmt (SPDLOG_FMT_EXTERNAL=1) and requires
  # explicit linking against libfmt. Ubuntu 20.04 bundles fmt inside spdlog, so
  # libfmt-dev is absent and find_package(fmt) would fail — use QUIET + guard.
  find_package(fmt QUIET)
  if(fmt_FOUND)
    # Modern CMake's fmt does not set fmt_LIBRARIES; set it manually so that
    # catkin_package(DEPENDS fmt) can propagate fmt::fmt to downstream packages.
    set(fmt_LIBRARIES fmt::fmt)
  endif()

  # Auto-generate ROS 2-style compat headers for standard msg packages.
  # Add/remove packages from this list as needed.
  set(_COMPAT_MSG_PACKAGES std_msgs geometry_msgs nav_msgs sensor_msgs
                           visualization_msgs diagnostic_msgs)

  set(_COMPAT_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/compat_include")
  set(_COMPAT_SCRIPT
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_ros1_compat_header.py")

  foreach(_pkg ${_COMPAT_MSG_PACKAGES})
    find_package(${_pkg} QUIET)
    if(${_pkg}_FOUND)
      set(_msg_dir "${${_pkg}_DIR}/../msg")
      if(EXISTS "${_msg_dir}")
        file(
          GLOB _msg_files
          RELATIVE "${_msg_dir}"
          "${_msg_dir}/*.msg")
        set(_msg_names "")
        foreach(_f ${_msg_files})
          get_filename_component(_n "${_f}" NAME_WE)
          list(APPEND _msg_names "${_n}")
        endforeach()
        if(_msg_names)
          execute_process(
            COMMAND
              "${PYTHON_EXECUTABLE}" "${_COMPAT_SCRIPT}" --package "${_pkg}"
              --output-dir "${_COMPAT_INCLUDE_DIR}" --messages ${_msg_names}
            RESULT_VARIABLE _result)
          if(_result EQUAL 0)
            message(STATUS "  Generated compat headers for ${_pkg}")
          endif()
        endif()
      endif()
    endif()
  endforeach()

  # The gtest_main helper is only built (and only useful) when testing
  # is enabled. Listing it conditionally in catkin_package avoids advertising
  # a non-existent library when CATKIN_ENABLE_TESTING is off.
  set(_COMPAT_EXPORT_LIBS "")
  if(CATKIN_ENABLE_TESTING)
    list(APPEND _COMPAT_EXPORT_LIBS sq_ros1_rclcpp_compat_gtest_main)
  endif()

  set(_COMPAT_DEPENDS spdlog)
  if(fmt_FOUND)
    list(APPEND _COMPAT_DEPENDS fmt)
  endif()

  catkin_package(
    INCLUDE_DIRS
    include
    ${_COMPAT_INCLUDE_DIR}
    LIBRARIES
    ${_COMPAT_EXPORT_LIBS}
    DEPENDS
    ${_COMPAT_DEPENDS}
    CFG_EXTRAS
    generate_ros1_compat_headers.cmake
    ros1_compat_defines.cmake)

  include_directories(include ${_COMPAT_INCLUDE_DIR} ${catkin_INCLUDE_DIRS})

  if(CATKIN_ENABLE_TESTING)
    find_package(rostest REQUIRED)
    find_package(GTest REQUIRED)

    # gtest_main equivalent for downstream tests. catkin_add_gtest does
    # not link gtest_main, so packages either roll their own main() or
    # link this library. It also calls ros::Time::init() so tests can
    # use rclcpp::Clock(RCL_ROS_TIME).now() (compat -> ros::Time::now())
    # without extra setup.
    add_library(sq_ros1_rclcpp_compat_gtest_main STATIC src/gtest_main.cpp)
    target_link_libraries(sq_ros1_rclcpp_compat_gtest_main
                          ${GTEST_LIBRARIES} ${catkin_LIBRARIES})
    install(TARGETS sq_ros1_rclcpp_compat_gtest_main
            ARCHIVE DESTINATION ${CATKIN_GLOBAL_LIB_DESTINATION})

    set(_TEST_FMT_LIB "")
    if(fmt_FOUND)
      set(_TEST_FMT_LIB fmt::fmt)
    endif()

    add_rostest_gtest(test_compat_basics test/test_compat_basics.test test/test_compat_basics.cpp)
    target_link_libraries(test_compat_basics ${catkin_LIBRARIES} ${_TEST_FMT_LIB})

    add_rostest_gtest(test_compat_node test/test_compat_node.test test/test_compat_node.cpp)
    target_link_libraries(test_compat_node ${catkin_LIBRARIES} ${_TEST_FMT_LIB})
  endif()

  install(DIRECTORY include/ DESTINATION ${CATKIN_GLOBAL_INCLUDE_DESTINATION})

elseif($ENV{ROS_VERSION} EQUAL 2)
  message(STATUS "Building sq_ros1_rclcpp_compat for ROS 2 (no-op)")

  find_package(ament_cmake REQUIRED)
  ament_package()

else()
  message(
    FATAL_ERROR "ROS_VERSION environment variable not set or unsupported.")
endif()
