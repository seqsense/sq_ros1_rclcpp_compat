# Copyright 2026 SEQSENSE, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# CMake function to convert ROS 2-notation interface definitions to ROS 1
# notation for catkin's message generator. Exported via CFG_EXTRAS so it is
# available to downstream catkin packages.
#
# The canonical source .msg/.srv/.action files use ROS 2 notation
# (std_msgs/Header, builtin_interfaces/Time, builtin_interfaces/Duration) so
# that the ROS 2 side needs no rewriting. For the ROS 1 (catkin) build this
# rewrites the two type tokens that ROS 1 spells differently:
#   builtin_interfaces/Time     -> time
#   builtin_interfaces/Duration -> duration
# std_msgs/Header is accepted verbatim by ROS 1, so it is left untouched.
#
# The match targets the fully-qualified ROS 2 type token, never the bare words
# "time"/"duration". This is deliberately the reverse of a ROS1->ROS2 rewrite:
# a field name that merely ends in "time"/"duration" (e.g. `stuck_started_since`,
# `has_watchdog_timed_out`) can never contain "builtin_interfaces/...", so there
# is no risk of clobbering field names.
#
# Usage (ROS 1 branch of a hybrid message package):
#   find_package(catkin REQUIRED COMPONENTS ... sq_ros1_rclcpp_compat)
#   preprocess_ros1_interfaces(SUBDIR msg FILES ${MSG_FILES}
#                              OUTPUT_DIR_VAR ROS1_MSG_DIR)
#   add_message_files(DIRECTORY ${ROS1_MSG_DIR} FILES ${MSG_FILES})
#
# Only needed for packages whose interfaces reference builtin_interfaces/Time or
# builtin_interfaces/Duration. Packages that only use std_msgs/Header can feed
# the source directory to add_message_files() directly.
function(preprocess_ros1_interfaces)
  cmake_parse_arguments(ARG "" "SUBDIR;OUTPUT_DIR_VAR" "FILES" ${ARGN})

  if(NOT ARG_SUBDIR)
    message(FATAL_ERROR "preprocess_ros1_interfaces: SUBDIR is required")
  endif()
  if(NOT ARG_OUTPUT_DIR_VAR)
    message(FATAL_ERROR "preprocess_ros1_interfaces: OUTPUT_DIR_VAR is required")
  endif()

  set(_out "${CMAKE_CURRENT_BINARY_DIR}/ros1_interfaces/${ARG_SUBDIR}")
  file(MAKE_DIRECTORY "${_out}")

  foreach(_file ${ARG_FILES})
    set(_src "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_SUBDIR}/${_file}")
    # The rewrite runs at configure time, so re-run configure (regenerating the
    # rewritten copy) whenever a source interface file's content changes;
    # otherwise the copy fed to the message generator could go stale.
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_src}")
    file(READ "${_src}" _content)
    string(REPLACE "builtin_interfaces/Time" "time" _content "${_content}")
    string(REPLACE "builtin_interfaces/Duration" "duration" _content "${_content}")
    file(WRITE "${_out}/${_file}" "${_content}")
  endforeach()

  set(${ARG_OUTPUT_DIR_VAR} "${_out}" PARENT_SCOPE)
endfunction()
