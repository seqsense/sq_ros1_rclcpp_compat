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

  # Aliases for the core ROS 1 interfaces live in include/, committed rather
  # than generated here; see scripts/regen_interface_aliases.py. They are pure
  # text over a package name and a type name, so building them needs none of
  # the interface packages -- which is why this package depends on none of
  # them. Their #include resolves in the consumer's compilation, against the
  # interface package that consumer already declares.

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
    LIBRARIES
    ${_COMPAT_EXPORT_LIBS}
    DEPENDS
    ${_COMPAT_DEPENDS}
    CFG_EXTRAS
    generate_ros1_compat_headers.cmake
    preprocess_ros1_interfaces.cmake
    ros1_compat_defines.cmake)

  include_directories(include ${catkin_INCLUDE_DIRS})

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
