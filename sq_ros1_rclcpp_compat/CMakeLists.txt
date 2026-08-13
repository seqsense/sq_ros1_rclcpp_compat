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

  # Auto-generate ROS 2-style compat headers for the core ROS 1 interfaces:
  # std_msgs, std_srvs, the common_msgs stack and tf2_msgs. Extend the list
  # deliberately -- and keep package.xml's build_depend entries in step, or
  # configuring fails below. An alias for a type ROS 2 does not have is a trap.
  #
  # These are build_depend rather than depend on purpose: they are needed here
  # only to enumerate type names. The headers written from that enumeration
  # are pure text, and the #include each emits resolves in the consumer's
  # compilation, against the interface package that consumer already declares
  # for itself. Declaring them any more strongly would push all twelve onto
  # every ROS 1 consumer of this package.
  set(_COMPAT_INTERFACE_PACKAGES
      actionlib_msgs
      diagnostic_msgs
      geometry_msgs
      nav_msgs
      sensor_msgs
      shape_msgs
      std_msgs
      std_srvs
      stereo_msgs
      tf2_msgs
      trajectory_msgs
      visualization_msgs)

  # Generated into the devel space rather than the build tree, so that the
  # headers can be installed. Downstream packages that consume an *installed*
  # sq_ros1_rclcpp_compat (an Alpine ROS apk, say) rather than a catkin devel
  # space would otherwise not find them.
  set(_COMPAT_INCLUDE_DIR
      "${CATKIN_DEVEL_PREFIX}/${CATKIN_GLOBAL_INCLUDE_DESTINATION}")
  set(_COMPAT_SCRIPT
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_ros1_compat_header.py")

  # Every step below fails loudly. Skipping one silently, as this loop used to
  # on a missing package, produces a package that builds green and ships an
  # incomplete set of headers -- a break that only surfaces much later, in an
  # unrelated package that cannot find a header this one was meant to provide.
  foreach(_pkg ${_COMPAT_INTERFACE_PACKAGES})
    find_package(${_pkg} REQUIRED)
    execute_process(
      COMMAND
        "${PYTHON_EXECUTABLE}" "${_COMPAT_SCRIPT}" --package "${_pkg}"
        --share-dir "${${_pkg}_DIR}/.."
        --output-dir "${_COMPAT_INCLUDE_DIR}"
      RESULT_VARIABLE _result
      OUTPUT_VARIABLE _output)
    if(NOT _result EQUAL 0)
      message(FATAL_ERROR "failed to generate compat headers for ${_pkg}")
    endif()
    string(STRIP "${_output}" _output)
    message(STATUS "  Generated compat headers for ${_output}")
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
    preprocess_ros1_interfaces.cmake
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
  install(DIRECTORY ${_COMPAT_INCLUDE_DIR}/
          DESTINATION ${CATKIN_GLOBAL_INCLUDE_DESTINATION})

elseif($ENV{ROS_VERSION} EQUAL 2)
  message(STATUS "Building sq_ros1_rclcpp_compat for ROS 2 (no-op)")

  find_package(ament_cmake REQUIRED)

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
  endif()

  ament_package()

else()
  message(
    FATAL_ERROR "ROS_VERSION environment variable not set or unsupported.")
endif()
