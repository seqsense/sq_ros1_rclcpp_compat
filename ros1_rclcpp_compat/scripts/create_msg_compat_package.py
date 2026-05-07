#!/usr/bin/env python3
"""Generate a ROS 2-style message compatibility package for a ROS 1 message package.

Creates a thin hybrid (ROS 1 / ROS 2) catkin/ament package that:
  - ROS 1: generates <pkg>/msg/<snake_case>.hpp compatibility headers
           using generate_ros1_compat_headers() from ros1_rclcpp_compat
  - ROS 2: no-op ament_cmake package (headers are already provided by rosidl)

Usage:
    python3 create_msg_compat_package.py sensor_msgs --output-dir /path/to/ws/src/
    python3 create_msg_compat_package.py geometry_msgs nav_msgs --output-dir /path/to/ws/src/
"""
import argparse
import os

CMAKELISTS_TEMPLATE = '''\
cmake_minimum_required(VERSION 3.5)
project({compat_name})

if($ENV{{ROS_VERSION}} EQUAL 1)
  message(STATUS "Building {compat_name} for ROS 1")

  find_package(catkin REQUIRED COMPONENTS {msg_package} ros1_rclcpp_compat)

  # Generate compat headers into a directory we can export via INCLUDE_DIRS
  set(_COMPAT_INCLUDE_DIR "${{CMAKE_CURRENT_BINARY_DIR}}/compat_include")
  generate_ros1_compat_headers(
    PACKAGE {msg_package}
    MSG_DIR "${{{msg_package}_DIR}}/../msg"
    OUTPUT_DIR "${{_COMPAT_INCLUDE_DIR}}"
  )

  catkin_package(INCLUDE_DIRS ${{_COMPAT_INCLUDE_DIR}})

elseif($ENV{{ROS_VERSION}} EQUAL 2)
  message(STATUS "Building {compat_name} for ROS 2 (no-op)")

  find_package(ament_cmake REQUIRED)
  ament_package()

else()
  message(FATAL_ERROR "ROS_VERSION environment variable not set or unsupported.")
endif()
'''

PACKAGE_XML_TEMPLATE = '''\
<?xml version="1.0"?>
<?xml-model href="http://download.ros.org/schema/package_format3.xsd"
            schematypens="http://www.w3.org/2001/XMLSchema"?>
<package format="3">
  <name>{compat_name}</name>
  <version>0.0.1</version>
  <description>ROS 2-style compatibility headers for {msg_package} on ROS 1. No-op on ROS 2.</description>
  <maintainer email="user@example.com">User</maintainer>
  <license>Apache-2.0</license>

  <buildtool_depend>cmake</buildtool_depend>
  <buildtool_depend condition="$ROS_VERSION == 1">catkin</buildtool_depend>
  <buildtool_depend condition="$ROS_VERSION == 2">ament_cmake</buildtool_depend>

  <build_depend condition="$ROS_VERSION == 1">{msg_package}</build_depend>
  <build_depend condition="$ROS_VERSION == 1">ros1_rclcpp_compat</build_depend>

  <export>
    <build_type condition="$ROS_VERSION == 2">ament_cmake</build_type>
  </export>
</package>
'''


def create_package(msg_package: str, output_dir: str) -> str:
    compat_name = f'{msg_package}_compat'
    pkg_dir = os.path.join(output_dir, compat_name)
    os.makedirs(pkg_dir, exist_ok=True)

    cmake_path = os.path.join(pkg_dir, 'CMakeLists.txt')
    with open(cmake_path, 'w') as f:
        f.write(CMAKELISTS_TEMPLATE.format(
            compat_name=compat_name, msg_package=msg_package))

    xml_path = os.path.join(pkg_dir, 'package.xml')
    with open(xml_path, 'w') as f:
        f.write(PACKAGE_XML_TEMPLATE.format(
            compat_name=compat_name, msg_package=msg_package))

    return pkg_dir


def main() -> None:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        'msg_packages', nargs='+',
        help='ROS message package names (e.g. sensor_msgs geometry_msgs)')
    parser.add_argument(
        '--output-dir', required=True,
        help='Directory to create compat packages in')
    args = parser.parse_args()

    for pkg in args.msg_packages:
        path = create_package(pkg, args.output_dir)
        print(f'Created: {path}/')


if __name__ == '__main__':
    main()
