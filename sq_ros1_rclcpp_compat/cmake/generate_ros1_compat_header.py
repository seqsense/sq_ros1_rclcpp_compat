#!/usr/bin/env python3
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
"""
Generate ROS 2-style compatibility headers for ROS 1 interface types.

For each ROS 1 message (e.g. hybrid_package_msgs/StampedPointer),
generates a header at <package>/msg/<snake_case>.hpp that:
  - Includes the original ROS 1 header
  - Creates a using-alias: <package>::msg::<MsgName> -> <package>::<MsgName>

Services work the same way, at <package>/srv/<snake_case>.hpp. The ROS 1
service class carries Request/Response member typedefs, matching how ROS 2
code spells <package>::srv::<SrvName>::Request.

The two namespaces refer to the same C++ type, so all message-traits,
container element types, and pub/sub APIs work identically whether code
spells the type with or without the ::msg:: segment.

The shared_ptr typedefs the ROS 1 generator emits (Ptr / ConstPtr,
boost-based) and the ROS 2 ones (SharedPtr / ConstSharedPtr, std-based)
are intentionally NOT re-exposed under ::msg:: — the two share_ptr
flavors are different types, and conflating them via member typedefs
creates surprising signatures across builds. Hybrid logic-layer code
should declare shared_ptr arguments as std::shared_ptr<const T> and let
the ROS 1 interface layer convert from boost::shared_ptr via
sq_ros1_compat::to_std() (see sq_ros1_compat/msg_ptr.hpp).
"""
import argparse
import os
import re

TEMPLATE = """\
// Copyright 2026 SEQSENSE, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef {guard}
#define {guard}

// Auto-generated ROS 2-style compatibility header for ROS 1.
// Provides: {package}::{kind}::{name} as a using-alias for the ROS 1
// type {package}::{name}. Logic-layer code that declares shared_ptr
// arguments as std::shared_ptr<const T> works on both builds; on ROS 1
// the IF layer converts boost::shared_ptr -> std::shared_ptr via
// sq_ros1_compat::to_std() (no deep copy).

#include "{package}/{name}.h"

namespace {package}
{{
namespace {kind}
{{
using {name} = ::{package}::{name};
}}  // namespace {kind}
}}  // namespace {package}

#endif  // {guard}
"""


def camel_to_snake(name: str) -> str:
    s1 = re.sub(r'(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', s1).lower()


def header_guard(package: str, msg_name: str, kind: str = 'msg') -> str:
    """Include guard cpplint derives from the header's path, not its type name."""
    return (f'{package.upper()}__{kind.upper()}'
            f'__{camel_to_snake(msg_name).upper()}_HPP_')


def render(package: str, msg_name: str, kind: str = 'msg') -> str:
    return TEMPLATE.format(
        package=package, name=msg_name, kind=kind,
        guard=header_guard(package, msg_name, kind))


def generate(package: str, msg_name: str, output_dir: str, kind: str = 'msg') -> str:
    snake = camel_to_snake(msg_name)

    out_path = os.path.join(output_dir, package, kind, f'{snake}.hpp')
    os.makedirs(os.path.dirname(out_path), exist_ok=True)

    content = render(package, msg_name, kind)

    # Only write if content changed to avoid unnecessary rebuilds
    if os.path.exists(out_path):
        with open(out_path) as f:
            if f.read() == content:
                return out_path

    with open(out_path, 'w') as f:
        f.write(content)
    return out_path


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--package', required=True, help='Package name')
    parser.add_argument('--output-dir', required=True, help='Base output directory')
    parser.add_argument(
        '--messages',
        nargs='+',
        default=[],
        help='CamelCase message names (e.g. StampedPointer)',
    )
    parser.add_argument(
        '--services',
        nargs='+',
        default=[],
        help='CamelCase service names (e.g. ResizeParticle)',
    )
    args = parser.parse_args()

    for name in args.messages:
        print(f'  Generated: {generate(args.package, name, args.output_dir, "msg")}')
    for name in args.services:
        print(f'  Generated: {generate(args.package, name, args.output_dir, "srv")}')


if __name__ == '__main__':
    main()
