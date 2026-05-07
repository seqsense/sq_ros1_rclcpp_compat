#!/usr/bin/env python3
"""Generate ROS 2-style compatibility headers for ROS 1 message types.

For each ROS 1 message (e.g. hybrid_package_msgs/StampedPointer),
generates a header at <package>/msg/<snake_case>.hpp that:
  - Includes the original ROS 1 header
  - Creates a namespace alias: <package>::msg::<MsgName> -> <package>::<MsgName>

This allows application code to use the ROS 2 include/namespace style
uniformly across both ROS 1 and ROS 2 builds.
"""
import argparse
import os
import re

TEMPLATE = '''\
#ifndef {guard}
#define {guard}

// Auto-generated ROS 2-style compatibility header for ROS 1
// Provides: {package}::msg::{msg_name}  with SharedPtr / ConstSharedPtr

#include "{package}/{msg_name}.h"

namespace {package} {{
namespace msg {{

struct {msg_name} : public ::{package}::{msg_name} {{
  using ::{package}::{msg_name}::{msg_name};
  {msg_name}() = default;
  {msg_name}(const ::{package}::{msg_name}& other)  // NOLINT
  : ::{package}::{msg_name}(other) {{}}

  using SharedPtr = ::{package}::{msg_name}::Ptr;
  using ConstSharedPtr = ::{package}::{msg_name}::ConstPtr;
}};

}}  // namespace msg
}}  // namespace {package}

#endif  // {guard}
'''


def camel_to_snake(name: str) -> str:
    s1 = re.sub(r'(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', s1).lower()


def generate(package: str, msg_name: str, output_dir: str) -> str:
    snake = camel_to_snake(msg_name)
    guard = f'{package.upper()}__MSG__{msg_name.upper()}_HPP_'

    out_path = os.path.join(output_dir, package, 'msg', f'{snake}.hpp')
    os.makedirs(os.path.dirname(out_path), exist_ok=True)

    content = TEMPLATE.format(package=package, msg_name=msg_name, guard=guard)

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
        required=True,
        help='CamelCase message names (e.g. StampedPointer)',
    )
    args = parser.parse_args()

    for msg in args.messages:
        path = generate(args.package, msg, args.output_dir)
        print(f'  Generated: {path}')


if __name__ == '__main__':
    main()
