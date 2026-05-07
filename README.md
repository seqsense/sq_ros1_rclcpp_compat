# sq_ros1_rclcpp_compat

Japanese version: [README_ja.md](README_ja.md)

ROS 2 `rclcpp` API shim for ROS 1, plus rclcpp-style headers for common message packages. Lets the same C++ source build on ROS 1 (Noetic) and ROS 2 (Jazzy / Humble) — on ROS 2 the package is intentionally a no-op.

The intended use case is **gradual ROS 1 → ROS 2 migration**: keep nodes building on ROS 1 while the logic layer is rewritten against the ROS 2 API. The shim is consumed by the migration agents in [sq_ros_hybrid_kit](https://github.com/seqsense/sq_ros_hybrid_kit), but it can also be dropped into any catkin workspace as a standalone package.

## What's provided

- **`rclcpp` shim** ([sq_ros1_rclcpp_compat/include/rclcpp/](sq_ros1_rclcpp_compat/include/rclcpp/)) — `rclcpp::Logger`, `rclcpp::Clock` / `Time` / `Duration`, `rclcpp::Node` (parameter API), and `RCLCPP_DEBUG/INFO/WARN/ERROR` plus their `_THROTTLE` variants. Logging is backed by `spdlog`; throttle uses `std::chrono::steady_clock`.
- **Auto-generated msg compat headers** — at build time `<pkg>/msg/<Type>.hpp` style headers are generated for `std_msgs`, `geometry_msgs`, `nav_msgs`, `sensor_msgs`, `visualization_msgs`, `diagnostic_msgs`. Add your own message packages via the `generate_ros1_compat_headers()` CMake helper exported by this package.
- **`tf2_*` ROS 2-style headers** ([tf2_eigen/](sq_ros1_rclcpp_compat/include/tf2_eigen/), [tf2_geometry_msgs/](sq_ros1_rclcpp_compat/include/tf2_geometry_msgs/), [tf2_sensor_msgs/](sq_ros1_rclcpp_compat/include/tf2_sensor_msgs/)) — let ROS 1 code use the ROS 2 include layout.
- **`sensor_msgs::PointCloud2Iterator`** ([sensor_msgs/](sq_ros1_rclcpp_compat/include/sensor_msgs/)) — header-only port of the ROS 2 iterator.
- **`sq_ros1_rclcpp_compat_gtest_main`** — a static lib that supplies `main()` and calls `ros::Time::init()`, so unit tests can use `rclcpp::Clock(RCL_ROS_TIME).now()` symmetrically across ROS 1 / ROS 2 with no test-side `#ifdef`. Linked transitively via `${catkin_LIBRARIES}` once the test target depends on this package.

On ROS 2 every `<depend>` and feature is gated on `condition="$ROS_VERSION == 1"`, and `ament_package()` is invoked with no targets.

## Repository layout

```
sq_ros1_rclcpp_compat/
├── sq_ros1_rclcpp_compat/   # the shim package itself (catkin / ament_cmake)
│   ├── include/             # public headers
│   ├── cmake/               # generate_ros1_compat_headers helper
│   ├── scripts/             # standalone helper for generating per-package compat packages
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
#include <std_msgs/msg/string.hpp>      // auto-generated compat header

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

- The shim covers the rclcpp surface that the migration samples actually exercise (logging, parameters, time, common msg types, tf2). It is **not** a complete `rclcpp` reimplementation — services, actions, lifecycle nodes, QoS profiles, and callback groups are intentionally not provided.
- API additions are welcome; **changing an existing API is not**. Once a signature ships, divergence between the shim and real `rclcpp` undermines the hybrid build premise.
- Node implementations targeted by the hybrid pattern are C++ only. Python nodes are out of scope.

## License

Apache-2.0. See [LICENSE](LICENSE).
