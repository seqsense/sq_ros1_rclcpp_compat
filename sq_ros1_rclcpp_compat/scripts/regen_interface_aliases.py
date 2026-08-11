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
Regenerate the committed ROS 2-style aliases for the core ROS 1 interfaces.

This is a maintenance tool, not part of the build. The aliases it writes are
committed under include/, alongside the hand-written forwarding headers, and
shipped by the plain install(DIRECTORY include/) rule.

Keeping generation out of the build is what lets this package declare no
dependency on any of the interface packages below. An alias header is pure
text derived from a package name and a type name -- it needs neither the
message definition nor the defining package to be written. The #include it
emits resolves in the *consumer's* compilation, against the interface package
that consumer already declares for itself.

Run it against any environment with the core interfaces installed:

    python3 scripts/regen_interface_aliases.py --ros-prefix /opt/ros/noetic
    python3 scripts/regen_interface_aliases.py --check    # CI / pre-commit

ROS 1 is end-of-life, so these definitions are frozen; a re-run is only
needed when the package list below changes.
"""
import argparse
import importlib.util
import os
import sys

# Core ROS 1 interface packages: std_msgs, std_srvs, the common_msgs stack
# and tf2_msgs. Extend deliberately -- every entry costs committed files and
# nothing else, but an alias for a type ROS 2 does not have is a trap.
PACKAGES = [
    'actionlib_msgs',
    'diagnostic_msgs',
    'geometry_msgs',
    'nav_msgs',
    'sensor_msgs',
    'shape_msgs',
    'std_msgs',
    'std_srvs',
    'stereo_msgs',
    'tf2_msgs',
    'trajectory_msgs',
    'visualization_msgs',
]

_HERE = os.path.dirname(os.path.abspath(__file__))
_PKG_ROOT = os.path.dirname(_HERE)


def _load_generator():
    path = os.path.join(_PKG_ROOT, 'cmake', 'generate_ros1_compat_header.py')
    spec = importlib.util.spec_from_file_location('gen_compat_header', path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def action_generated(share_dir):
    """Names genaction.py derives from *.action, which have no ROS 2 analogue.

    ROS 1 expands Foo.action into seven flat messages; ROS 2 models the same
    thing as a single action type with nested Goal/Result/Feedback. Aliasing
    e.g. nav_msgs::msg::GetMapAction would promise a type ROS 2 never defines.
    """
    excluded = set()
    action_dir = os.path.join(share_dir, 'action')
    if not os.path.isdir(action_dir):
        return excluded
    for entry in sorted(os.listdir(action_dir)):
        if not entry.endswith('.action'):
            continue
        stem = entry[: -len('.action')]
        excluded.add(f'{stem}Action')
        for suffix in ('Goal', 'Result', 'Feedback'):
            excluded.add(f'{stem}Action{suffix}')
            excluded.add(f'{stem}{suffix}')
    return excluded


def collect(share_dir, kind):
    """CamelCase type names for kind ('msg' or 'srv'), action spillover removed."""
    type_dir = os.path.join(share_dir, kind)
    if not os.path.isdir(type_dir):
        return []
    ext = '.' + kind
    excluded = action_generated(share_dir) if kind == 'msg' else set()
    names = [
        entry[: -len(ext)]
        for entry in os.listdir(type_dir)
        if entry.endswith(ext)
    ]
    return sorted(n for n in names if n not in excluded)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        '--ros-prefix',
        default=os.environ.get('ROS_PREFIX', '/opt/ros/noetic'),
        help='ROS 1 install prefix holding share/<package>/{msg,srv}',
    )
    parser.add_argument(
        '--output-dir',
        default=os.path.join(_PKG_ROOT, 'include'),
        help='Where to write the aliases (default: this package\'s include/)',
    )
    parser.add_argument(
        '--check',
        action='store_true',
        help='Do not write; exit non-zero if anything is missing or stale',
    )
    args = parser.parse_args()

    generator = _load_generator()

    missing_packages = []
    stale = []
    written = 0

    for package in PACKAGES:
        share_dir = os.path.join(args.ros_prefix, 'share', package)
        if not os.path.isdir(share_dir):
            missing_packages.append(package)
            continue
        for kind in ('msg', 'srv'):
            for name in collect(share_dir, kind):
                out_path = os.path.join(
                    args.output_dir, package, kind,
                    generator.camel_to_snake(name) + '.hpp')
                if args.check:
                    expected = generator.TEMPLATE.format(
                        package=package, name=name, kind=kind,
                        guard=f'{package.upper()}__{kind.upper()}'
                              f'__{name.upper()}_HPP_')
                    if not os.path.exists(out_path):
                        stale.append(f'{out_path} (missing)')
                    elif open(out_path).read() != expected:
                        stale.append(f'{out_path} (differs)')
                    continue
                generator.generate(package, name, args.output_dir, kind)
                written += 1
        print(f'  {package}: '
              f'{len(collect(share_dir, "msg"))} msg, '
              f'{len(collect(share_dir, "srv"))} srv')

    if missing_packages:
        print(
            f'error: not installed under {args.ros_prefix}: '
            f'{" ".join(missing_packages)}',
            file=sys.stderr)
        return 1

    if args.check:
        if stale:
            print('error: committed aliases are out of date:', file=sys.stderr)
            for entry in stale:
                print(f'  {entry}', file=sys.stderr)
            return 1
        print('aliases are up to date')
        return 0

    print(f'wrote {written} headers to {args.output_dir}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
