# sq_ros1_rclcpp_compat

Japanese version: [README_ja.md](README_ja.md)

ROS 2 `rclcpp` API shim for ROS 1, plus rclcpp-style headers for common message packages. Lets the same C++ source build on ROS 1 (Noetic) and ROS 2 (Jazzy / Humble) — on ROS 2 the package is intentionally a no-op.

The intended use case is **gradual ROS 1 → ROS 2 migration**: keep nodes building on ROS 1 while the logic layer is rewritten against the ROS 2 API.

[sq_ros_hybrid_kit](https://github.com/seqsense/sq_ros_hybrid_kit) builds on this shim with AI agents that turn existing ROS 1 packages into hybrid packages that build and run on both ROS 1 and ROS 2, plus a Docker environment for building and running them on either side.

## Hybridization design philosophy

A hybrid C++ package is structured in three layers:

- **Logic layer** — pure C++ written against `rclcpp`. One `.cpp` / `.hpp` set is shared between the two builds; gtests sit on top of the same source and are shared too.
- **ROS 1 interface layer** — written against `roscpp`.
- **ROS 2 interface layer** — written against `rclcpp`.

The node-level interfaces of `roscpp` and `rclcpp` diverge too much to share, so the **interface layer is intentionally written twice**. Only the logic layer is shared, and this shim is what makes that practical: it exposes on top of `roscpp` the rclcpp surface the logic layer cannot avoid (`Logger`, `Clock` / `Time`, message types, `RCLCPP_*` macros, throttled logging, ...).

In practice, once the complex logic has been factored out and pinned down by unit tests, migrating the existing ROS 1 interface to its ROS 2 counterpart tends to be relatively straightforward.

The canonical embodiment is [`samples/hybrid_imu_analyzer/`](samples/hybrid_imu_analyzer/) — read its [`src/imu_analyzer.cpp`](samples/hybrid_imu_analyzer/src/imu_analyzer.cpp) (shared logic) alongside [`src/ros1_imu_analyzer_node.cpp`](samples/hybrid_imu_analyzer/src/ros1_imu_analyzer_node.cpp) and [`src/ros2_imu_analyzer_node.cpp`](samples/hybrid_imu_analyzer/src/ros2_imu_analyzer_node.cpp).

## What's provided

### `rclcpp` shim

`rclcpp::Logger`, `rclcpp::Clock` / `Time` / `Duration`, `rclcpp::Node` (parameter API), and `RCLCPP_DEBUG/INFO/WARN/ERROR` plus their `_THROTTLE` variants. Headers live under [sq_ros1_rclcpp_compat/include/rclcpp/](sq_ros1_rclcpp_compat/include/rclcpp/). Logging is backed by `spdlog`; throttle uses `std::chrono::steady_clock`.

### ROS 1 / ROS 2 message-header alignment

ROS 2 uses `#include <std_msgs/msg/string.hpp>` and the type `std_msgs::msg::String`; ROS 1 uses `#include <std_msgs/String.h>` and `std_msgs::String`. To let the logic layer use the ROS 2 form on both sides, this package generates a shim header on ROS 1 — at `<pkg>/msg/<snake_case>.hpp` — that includes the ROS 1 header and re-exposes the type as `<pkg>::msg::<CamelCase>` via a using-alias to the ROS 1 type. The two namespaces refer to exactly the same C++ type, so message-traits, container element types, and pub/sub APIs work identically with or without the `::msg::` segment.

The shared_ptr typedefs the ROS 1 generator emits (`Ptr` / `ConstPtr`, `boost::shared_ptr`-based) and the ROS 2 ones (`SharedPtr` / `ConstSharedPtr`, `std::shared_ptr`-based) are intentionally *not* re-exposed under `::msg::` — the two flavors are different types, and conflating them via member typedefs makes shared_ptr signatures behave inconsistently across builds. Hybrid logic-layer code declares shared_ptr arguments as `std::shared_ptr<const T>`; the ROS 1 IF layer converts from `boost::shared_ptr` via `sq_ros1_compat::to_std()` (see below) — an aliasing-constructor wrap, not a deep copy.

Six standard packages (`std_msgs`, `geometry_msgs`, `nav_msgs`, `sensor_msgs`, `visualization_msgs`, `diagnostic_msgs`) are handled out of the box. For your own message packages, call `generate_ros1_compat_headers()` from your `CMakeLists.txt` — see [`samples/hybrid_package_msgs/`](samples/hybrid_package_msgs/) for the recipe.

### ROS 1 interface boundary helpers (`sq_ros1_compat/`)

Headers under [sq_ros1_rclcpp_compat/include/sq_ros1_compat/](sq_ros1_rclcpp_compat/include/sq_ros1_compat/) sit outside the `rclcpp/` shim tree on purpose: they expose no rclcpp surface and are meant to be included from ROS 1 native interface code.

| Header | Provides | Use |
|--------|----------|-----|
| `sq_ros1_compat/logger.hpp` | `sq_ros1_compat::get_logger(name)` | Construct the `rclcpp::Logger` to pass into a hybridized logic class. |
| `sq_ros1_compat/msg_ptr.hpp` | `sq_ros1_compat::to_std()` / `to_boost()` | Convert between `boost::shared_ptr` and `std::shared_ptr` at the IF boundary using each library's aliasing constructor — no deep copy. |

Convention for hybridized packages: a ROS 1 IF (`ros1_*.cpp`) only includes `ros/...`, the native msg header (`<msg_pkg>/<Type>.h`), the logic class header, and `sq_ros1_compat/*.hpp`. It does not pull in `rclcpp/...` directly. The logic class then receives the logger and `std::shared_ptr<const T>` messages from the IF without either side being aware of the other's shared_ptr library.

### `tf2_*` ROS 2-style headers

[tf2_eigen/](sq_ros1_rclcpp_compat/include/tf2_eigen/), [tf2_geometry_msgs/](sq_ros1_rclcpp_compat/include/tf2_geometry_msgs/), [tf2_sensor_msgs/](sq_ros1_rclcpp_compat/include/tf2_sensor_msgs/) — let ROS 1 code use the ROS 2 include layout.

### `sensor_msgs::PointCloud2Iterator`

Wrapper header at [sq_ros1_rclcpp_compat/include/sensor_msgs/](sq_ros1_rclcpp_compat/include/sensor_msgs/) that lets ROS 1 code use the ROS 2 include layout (`<sensor_msgs/point_cloud2_iterator.hpp>`); it forwards to the ROS 1 native `<sensor_msgs/point_cloud2_iterator.h>`.

### `sq_ros1_rclcpp_compat_gtest_main`

ROS 1's `catkin_add_gtest` does not link a `gtest_main`, so each test would normally need its own `main()`. This package supplies a static lib that provides `main()` and also calls `ros::Time::init()` once, letting tests use `rclcpp::Clock(RCL_ROS_TIME).now()` with no per-file boilerplate. It is exported through `${catkin_LIBRARIES}`, so any test target that depends on this package picks it up transitively — no explicit `target_link_libraries(<test> sq_ros1_rclcpp_compat_gtest_main)` needed. On ROS 2 the shim is unused; `ament_add_gtest` already supplies `gtest_main`.

For a test that links it implicitly, see [`samples/hybrid_imu_analyzer/test/`](samples/hybrid_imu_analyzer/test/) and the ROS 1 branch of its [`CMakeLists.txt`](samples/hybrid_imu_analyzer/CMakeLists.txt).

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

Node implementations targeted by the hybrid pattern are C++ only. Python and other non-C++ languages are out of scope.

## License

Apache-2.0. See [LICENSE](LICENSE).
