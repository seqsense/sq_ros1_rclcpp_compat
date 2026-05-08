# hybrid_imu_analyzer — ROS1/ROS2 hybrid node sample

Post-migration reference for a single-node C++ package. The pre-migration ROS1-only counterpart lives at [`workspace/src/imu_analyzer/`](../../../workspace/src/imu_analyzer/); this directory is what the migration agents (`logic-interface-separation` → `migration-logic` → `migration-hybrid-interface`) aim to produce.

## What this sample demonstrates

- **Logic / interface separation** — `imu_analyzer.{hpp,cpp}` is pure algorithm with an `rclcpp::Logger` injected by constructor; it does not touch Pub/Sub or parameters. The `ros1_*` / `ros2_*` wrappers are the only files that do I/O.
- **Hybrid build** — `CMakeLists.txt` branches on `$ENV{ROS_VERSION}` (catkin vs. ament_cmake). `package.xml` uses format 3 with `condition="$ROS_VERSION == 1/2"` on every branched dependency.
- **ROS1 side** — uses `sq_ros1_rclcpp_compat`, the shim that provides ROS2-style (`rclcpp`) API on top of roscpp, so the logic class can be written once against rclcpp and run on both sides.
- **ROS2 side as a component** — no standalone `main`; the node inherits from `rclcpp::Node`, registers via `RCLCPP_COMPONENTS_REGISTER_NODE`, and is built with `ament_auto_add_library` + `rclcpp_components_register_node(... EXECUTABLE ...)` (the `EXECUTABLE` argument generates a standalone binary too).
- **Zero Copy intra-process convention** — ROS2 subscriber callbacks take `const MsgType::ConstSharedPtr &`; publishers publish `std::unique_ptr<MsgType>` via `std::move`. The hybrid logic class signature uses `std::shared_ptr<const T>` so it is callable from either side; the ROS1 IF converts the subscriber's `boost::shared_ptr` to `std::shared_ptr` via `sq_ros1_compat::to_std()` (aliasing constructor — no deep copy). See [`../hybrid_package_pubsub_samples/`](../hybrid_package_pubsub_samples/) for a full pub → sub pipeline across multiple nodes.

## File map

| File | Role |
|------|------|
| `include/hybrid_imu_analyzer/imu_analyzer.hpp`, `src/imu_analyzer.cpp` | Hybrid logic class (constructor takes `rclcpp::Logger`; no Pub/Sub) |
| `src/ros1_imu_analyzer_node.cpp` | ROS1 wrapper instantiating the logic class |
| `src/ros2_imu_analyzer_node.cpp` | ROS2 component wrapping the same logic class |
| `CMakeLists.txt` | `$ENV{ROS_VERSION}` branches — ROS1: catkin; ROS2: `ament_cmake_auto` + component registration |
| `package.xml` | Format 3, conditional dependencies on `$ROS_VERSION` |
| `launch/` | `.launch` (ROS1) and `.launch.py` (ROS2) |

## Referenced from

- `migration/agents/{ja,en}/migration-hybrid-interface.md` — canonical target shape for the ROS2 wrapper
- `migration/agents/{ja,en}/migration-logic.md` — canonical target shape for the hybrid logic library
