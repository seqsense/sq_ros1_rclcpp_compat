# sq_ros1_rclcpp_compat

Japanese version: [README_ja.md](README_ja.md)

A `rclcpp` (ROS 2) API shim for ROS 1, plus rclcpp-style headers for commonly used message packages.

The intended use case is **gradual ROS 1 → ROS 2 migration**: rewrite a node's logic layer against the ROS 2 API while keeping it building on ROS 1.

[sq_ros_hybrid_kit](https://github.com/seqsense/sq_ros_hybrid_kit) builds on this shim, providing AI agents that convert existing ROS 1 C++ packages into hybrid packages buildable on both ROS 1 (Noetic or ROS One) and ROS 2 (Humble or Jazzy), along with a Docker environment for building and running them on either side.

## Hybridization design philosophy

A hybrid C++ package is structured in three layers:

- **Shared ROS 1 / ROS 2 logic layer** — written against `rclcpp`
- **ROS 1 interface layer** — written against `roscpp`
- **ROS 2 interface layer** — written against `rclcpp`

The node-level interfaces of `roscpp` and `rclcpp` diverge too much to share, so the interface layer is written twice. Only the logic layer is shared, and this package is what makes that sharing practical: on top of `roscpp` it provides the rclcpp surface the logic layer cannot avoid (`Logger`, `Clock` / `Time`, message types, `RCLCPP_*` macros, throttled logging, ...) as a ROS 1 shim, along with message-type conversion wrappers.

Once the package has been verified on ROS 2 and ROS 1 support is no longer needed, it can be turned into a pure ROS 2 package simply by deleting the ROS 1 interface layer and the ROS 1-specific parts of `CMakeLists.txt` / `package.xml` (including the dependency on this package).

The canonical example is under [`samples/hybrid_imu_analyzer/`](samples/hybrid_imu_analyzer/) — reading the shared logic [`src/imu_analyzer.cpp`](samples/hybrid_imu_analyzer/src/imu_analyzer.cpp) alongside the two interface wrappers [`src/ros1_imu_analyzer_node.cpp`](samples/hybrid_imu_analyzer/src/ros1_imu_analyzer_node.cpp) / [`src/ros2_imu_analyzer_node.cpp`](samples/hybrid_imu_analyzer/src/ros2_imu_analyzer_node.cpp) is the clearest introduction.

## What's provided

### ROS 1 interface layer helpers

The headers under [sq_ros1_rclcpp_compat/include/sq_ros1_compat/](sq_ros1_rclcpp_compat/include/sq_ros1_compat/) are included from the ROS 1 interface layer to construct the objects handed to the shared logic layer.

| Header | Provides | Use |
|--------|----------|-----|
| `sq_ros1_compat/logger.hpp` | `sq_ros1_compat::get_logger(name)` | Construct the `rclcpp::Logger` passed to the shared logic layer |
| `sq_ros1_compat/msg_ptr.hpp` | `sq_ros1_compat::to_std()` / `to_boost()` | Convert message pointers between `boost::shared_ptr` and `std::shared_ptr` |

A shared logic-layer class is recommended to receive its `rclcpp::Logger` through the constructor (or similar). On a ROS 2 build this is the real `rclcpp::Logger`; on a ROS 1 build it is a shim object created by `sq_ros1_compat::get_logger(name)`, through which logging is emitted.

For ROS messages, the pointer types the ROS 1 generator produces (`Ptr` / `ConstPtr`) are `boost::shared_ptr`-based whereas the ROS 2 ones (`SharedPtr` / `ConstSharedPtr`) are `std::shared_ptr`-based, so they cannot be substituted directly. When using this package, the recommendation is to declare message pointers in the shared logic layer as `std::shared_ptr<T>` or `std::shared_ptr<const T>`, and to convert them from `boost::shared_ptr` on the ROS 1 interface side via `sq_ros1_compat::to_std()`. That function uses the aliasing constructor, so it converts the pointer without a deep copy.

### Shared ROS 1 / ROS 2 logic layer helpers

As noted above, the shared logic layer is written in ROS 2 (rclcpp) style. To build it on ROS 1, the following are provided.

#### `rclcpp` shim

Provides the rclcpp API used by the logic layer on ROS 1. Headers live under [sq_ros1_rclcpp_compat/include/rclcpp/](sq_ros1_rclcpp_compat/include/rclcpp/).

| Provides | Notes |
|----------|-------|
| `rclcpp::Logger` | Backed by `spdlog` |
| `RCLCPP_DEBUG/INFO/WARN/ERROR` and each `_THROTTLE` variant | Throttle is based on `std::chrono::steady_clock` |
| `rclcpp::Clock` / `Time` / `Duration` | Time / duration (per-clock-type backend noted below) |
| `rclcpp::Node` | Parameter API plus minimal helpers (`get_logger()` / `now()` / `get_name()`); no ROS 2 node functionality such as pub/sub |

The `rclcpp::Clock` backend depends on the clock type: `RCL_ROS_TIME` uses `ros::Time`, while `RCL_SYSTEM_TIME` / `RCL_STEADY_TIME` use `ros::WallTime`. Note that `ros::WallTime` is a wall clock, so — unlike the real rclcpp — `RCL_STEADY_TIME` does not guarantee monotonicity.

#### ROS 1 / ROS 2 message-header alignment

The include path and type name of a message differ between ROS 1 and ROS 2:

| | include | Type name |
|---|---------|-----------|
| ROS 1 | `#include "std_msgs/String.h"` | `std_msgs::String` |
| ROS 2 | `#include "std_msgs/msg/string.hpp"` | `std_msgs::msg::String` |

This package can generate wrapper headers that let the ROS 2 form be used from ROS 1: it generates a header named `<pkg>/msg/<snake_case>.hpp` that includes the original ROS 1 header and re-exposes `<pkg>::msg::<CamelCase>` as a using-alias to the ROS 1 type.

Six standard packages (`std_msgs`, `geometry_msgs`, `nav_msgs`, `sensor_msgs`, `visualization_msgs`, `diagnostic_msgs`) get their ROS 1 wrapper headers generated automatically within this package. For your own interface packages, call `generate_ros1_compat_headers()` from your `CMakeLists.txt` to generate the ROS 1 wrapper headers. See [samples/hybrid_package_msgs/](samples/hybrid_package_msgs/) for a concrete example.

Note that only a type using-alias is generated; pointer member types such as `SharedPtr` / `ConstSharedPtr` are not defined. A using-alias merely gives another name to the existing ROS 1 type and cannot add members to it, and defining a distinct type instead would break type identity and no longer match the pub/sub and message-traits specializations. In addition, a ROS 1 subscription callback delivers a `boost::shared_ptr`, so defining `std::shared_ptr`-based members would serve no purpose. As described above, the recommendation is to declare message pointers in the shared logic layer as `std::shared_ptr<T>` or `std::shared_ptr<const T>` and to convert them on the ROS 1 interface side via `sq_ros1_compat::to_std()`.

#### Rewriting interface definitions for ROS 1 (`preprocess_ros1_interfaces()`)

Author your interface definitions in the canonical ROS 2 notation as well, and use `preprocess_ros1_interfaces()` (described in this section) to rewrite them when building on ROS 1. There are only three built-in types whose notation differs between ROS 1 and ROS 2 and thus needs rewriting:

| ROS 1 notation | ROS 2 notation |
|----------------|----------------|
| `Header` | `std_msgs/Header` |
| `time` | `builtin_interfaces/Time` |
| `duration` | `builtin_interfaces/Duration` |

Every other built-in type (`bool` / `int8` / `uint8` / `float64` / `string`, arrays, `byte`, ...) and package-qualified `pkg/Type` types share the same notation in ROS 1 and ROS 2, so they need no rewriting.

Of the three, `std_msgs/Header` builds on both ROS 1 and ROS 2, so you only need to write `std_msgs/Header` in the source (in place of `Header`).

The remaining `builtin_interfaces/Time` / `builtin_interfaces/Duration` are not understood by the ROS 1 generator, so `preprocess_ros1_interfaces()` is provided to rewrite them back to the ROS 1 primitives (`time` / `duration`) at ROS 1 build time. Use it as follows:

```cmake
preprocess_ros1_interfaces(SUBDIR msg FILES ${MSG_FILES} OUTPUT_DIR_VAR ROS1_MSG_DIR)
add_message_files(DIRECTORY ${ROS1_MSG_DIR} FILES ${MSG_FILES})
```

| Keyword argument | Meaning |
|------------------|---------|
| `SUBDIR` | Name of the subdirectory holding the interface files (one of `msg` / `srv` / `action`) |
| `FILES` | List of interface file names |
| `OUTPUT_DIR_VAR` | Name of a variable that receives the path of the directory holding the rewritten files (the function defines a variable of this name in the caller's scope) |

Passing the rewritten directory (the variable received via `OUTPUT_DIR_VAR`) and the file names to `add_message_files` (likewise `add_service_files` / `add_action_files`) is what makes the ROS 1 build succeed.

#### `*.hpp` header wrappers

For the following, a header that was `*.h` on ROS 1 was renamed to `*.hpp` on ROS 2, so this package provides simple wrappers that just forward to the `*.h` header:

- [tf2_eigen/tf2_eigen.hpp](sq_ros1_rclcpp_compat/include/tf2_eigen/tf2_eigen.hpp)
- [tf2_geometry_msgs/tf2_geometry_msgs.hpp](sq_ros1_rclcpp_compat/include/tf2_geometry_msgs/tf2_geometry_msgs.hpp)
- [tf2_sensor_msgs/tf2_sensor_msgs.hpp](sq_ros1_rclcpp_compat/include/tf2_sensor_msgs/tf2_sensor_msgs.hpp)
- [sensor_msgs/point_cloud2_iterator.hpp](sq_ros1_rclcpp_compat/include/sensor_msgs/point_cloud2_iterator.hpp)

### `sq_ros1_rclcpp_compat_gtest_main`

ROS 1's `catkin_add_gtest` does not link a `gtest_main`, so each test would normally need its own `main()`. This package supplies a static lib that provides `main()` and also calls `ros::Time::init()` once, letting tests use `rclcpp::Clock(RCL_ROS_TIME).now()` with no per-file boilerplate. It is built and exported through `${catkin_LIBRARIES}` only when `CATKIN_ENABLE_TESTING` is on (i.e. during test builds); when it is, any test target that depends on this package picks it up transitively — no explicit `target_link_libraries(<test> sq_ros1_rclcpp_compat_gtest_main)` needed.

On ROS 2 this mechanism is unnecessary; `ament_add_gtest` already supplies `gtest_main`.

For a test that links it implicitly, see [`samples/hybrid_imu_analyzer/test/`](samples/hybrid_imu_analyzer/test/) and the ROS 1 branch of its [`CMakeLists.txt`](samples/hybrid_imu_analyzer/CMakeLists.txt).

## Repository layout

```
sq_ros1_rclcpp_compat/
├── sq_ros1_rclcpp_compat/   # the shim package itself (catkin / ament_cmake)
│   ├── include/             # public headers
│   ├── cmake/               # generate_ros1_compat_headers / preprocess_ros1_interfaces helpers
│   ├── scripts/             # script that generates a standalone compat package for a msg package not handled out of the box
│   ├── src/                 # gtest_main implementation
│   └── test/                # rostest unit tests for the shim
└── samples/
    ├── hybrid_imu_analyzer/             # logic / interface separation example
    ├── hybrid_package_msgs/             # hybrid msg package example
    └── hybrid_package_pubsub_samples/   # pub/sub + intra-process Zero Copy example
```

## Using it from your package

ROS 1 side (catkin):

```xml
<!-- package.xml -->
<depend condition="$ROS_VERSION == 1">sq_ros1_rclcpp_compat</depend>
```

```cmake
# CMakeLists.txt (ROS 1 branch)
find_package(catkin REQUIRED COMPONENTS sq_ros1_rclcpp_compat ...)
include_directories(${catkin_INCLUDE_DIRS})
target_link_libraries(<your_target> ${catkin_LIBRARIES})
```

```cpp
#include <rclcpp/rclcpp.hpp>            // resolves to the shim on ROS 1
#include <std_msgs/msg/string.hpp>      // auto-generated wrapper header

auto logger = rclcpp::get_logger("my_node");
RCLCPP_INFO(logger, "hello %d", 42);
```

The same source compiles unchanged on ROS 2 — `ament_cmake` resolves `<rclcpp/rclcpp.hpp>` to the real `rclcpp`, and the `condition="$ROS_VERSION == 1"` dependency on this package is skipped.

For working examples, see [`samples/`](samples/):

| Sample | What it demonstrates |
|--------|----------------------|
| [`hybrid_imu_analyzer/`](samples/hybrid_imu_analyzer/) | Logic / interface split with a ROS 1 wrapper and a ROS 2 component sharing one logic class |
| [`hybrid_package_msgs/`](samples/hybrid_package_msgs/) | Hybrid msg package using `generate_ros1_compat_headers()` |
| [`hybrid_package_pubsub_samples/`](samples/hybrid_package_pubsub_samples/) | Pub / sub pipeline plus intra-process Zero Copy convention |

## Building / testing as a standalone package

Drop the shim into a catkin source tree:

```bash
ln -s /path/to/sq_ros1_rclcpp_compat/sq_ros1_rclcpp_compat <workspace>/src/sq_ros1_rclcpp_compat
catkin build sq_ros1_rclcpp_compat
catkin run_tests sq_ros1_rclcpp_compat
```

Or use the Docker environment provided by [`sq_ros_hybrid_kit`](https://github.com/seqsense/sq_ros_hybrid_kit), which clones this repo, sets up the symlink, and exposes `make build-ros1 PKGS=sq_ros1_rclcpp_compat` / `make test-ros1`.

## Scope and limitations

Node implementations targeted by the hybrid pattern are C++ only. Python and other non-C++ languages are out of scope.

## License

Apache-2.0. See [LICENSE](LICENSE).
