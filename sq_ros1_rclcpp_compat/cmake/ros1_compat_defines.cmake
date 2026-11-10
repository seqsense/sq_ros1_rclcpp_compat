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

# Automatically define IS_ROS1_BUILD for all packages that depend on sq_ros1_rclcpp_compat.
# This allows shared headers to use #ifdef IS_ROS1_BUILD for ROS1-specific APIs
# (XmlRpc, dynamic_reconfigure, etc.) that have no compat shim equivalent.
add_compile_definitions(IS_ROS1_BUILD)
