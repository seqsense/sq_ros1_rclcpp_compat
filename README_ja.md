# sq_ros1_rclcpp_compat

English version: [README.md](README.md)

ROS 1 上で ROS 2 の `rclcpp` API を使うための shim ライブラリと、よく使うメッセージパッケージの rclcpp スタイルヘッダ群。同じ C++ ソースを ROS 1 (Noetic) と ROS 2 (Jazzy / Humble) の両方でビルドできる (ROS 2 上では意図的に no-op)。

想定ユースケースは **段階的な ROS 1 → ROS 2 移行**: ノードのロジック層を ROS 2 API に書き換えつつ、ROS 1 でもビルドし続けられるようにすること。本リポジトリの shim は [sq_ros_hybrid_kit](https://github.com/seqsense/sq_ros_hybrid_kit) のマイグレーションエージェントが利用することを主目的としているが、catkin ワークスペースへ単体パッケージとして投入することもできる。

## 提供物

- **`rclcpp` shim** ([sq_ros1_rclcpp_compat/include/rclcpp/](sq_ros1_rclcpp_compat/include/rclcpp/)) — `rclcpp::Logger`、`rclcpp::Clock` / `Time` / `Duration`、`rclcpp::Node` (パラメータ API)、`RCLCPP_DEBUG/INFO/WARN/ERROR` および各 `_THROTTLE` 版。ログ出力は `spdlog`、throttle は `std::chrono::steady_clock` ベース。
- **メッセージ compat ヘッダの自動生成** — ビルド時に `std_msgs` / `geometry_msgs` / `nav_msgs` / `sensor_msgs` / `visualization_msgs` / `diagnostic_msgs` について `<pkg>/msg/<Type>.hpp` 形式のヘッダを生成。独自パッケージは本パッケージが提供する CMake ヘルパ `generate_ros1_compat_headers()` で追加可能。
- **`tf2_*` の ROS 2 風ヘッダ** ([tf2_eigen/](sq_ros1_rclcpp_compat/include/tf2_eigen/)、[tf2_geometry_msgs/](sq_ros1_rclcpp_compat/include/tf2_geometry_msgs/)、[tf2_sensor_msgs/](sq_ros1_rclcpp_compat/include/tf2_sensor_msgs/)) — ROS 1 上で ROS 2 のインクルードレイアウトを利用可能に。
- **`sensor_msgs::PointCloud2Iterator`** ([sensor_msgs/](sq_ros1_rclcpp_compat/include/sensor_msgs/)) — ROS 2 のイテレータをヘッダオンリで移植。
- **`sq_ros1_rclcpp_compat_gtest_main`** — `main()` と `ros::Time::init()` を提供する static ライブラリ。テストコード側で `#ifdef` を書かずに `rclcpp::Clock(RCL_ROS_TIME).now()` を ROS 1 / ROS 2 の両方で対称に使える。テストターゲットが本パッケージに依存していれば `${catkin_LIBRARIES}` 経由で transitive にリンクされる。

ROS 2 側では `<depend>` と機能はすべて `condition="$ROS_VERSION == 1"` でゲートされており、`ament_package()` のみが空ターゲットで呼ばれる。

## ディレクトリ構成

```
sq_ros1_rclcpp_compat/
├── sq_ros1_rclcpp_compat/   # shim パッケージ本体 (catkin / ament_cmake)
│   ├── include/             # 公開ヘッダ
│   ├── cmake/               # generate_ros1_compat_headers ヘルパ
│   ├── scripts/             # 個別パッケージ向け compat パッケージ生成ヘルパ
│   ├── src/                 # gtest_main 実装
│   └── test/                # shim 自体の rostest ユニットテスト
└── samples/
    ├── hybrid_imu_analyzer/             # ロジック / インタフェース分離のサンプル
    ├── hybrid_package_msgs/             # ハイブリッド msg パッケージのサンプル
    └── hybrid_package_pubsub_samples/   # pub/sub + intra-process Zero Copy のサンプル
```

## 自パッケージから利用する

ROS 1 側 (catkin):

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
#include <rclcpp/rclcpp.hpp>            // ROS 1 では shim、ROS 2 では本物に解決
#include <std_msgs/msg/string.hpp>      // 自動生成された compat ヘッダ

auto logger = rclcpp::get_logger("my_node");
RCLCPP_INFO(logger, "hello %d", 42);
```

ROS 2 (Jazzy / Humble、`ament_cmake`) では同じソースが本物の `rclcpp` に解決され、本パッケージへの `condition="$ROS_VERSION == 1"` 依存はスキップされる。書き換え不要。

実例は [`samples/`](samples/) 配下を参照:

| サンプル | 何を示すか |
|----------|-----------|
| [`hybrid_imu_analyzer/`](samples/hybrid_imu_analyzer/) | ロジック / インタフェース分離。1 つのロジッククラスを ROS 1 ラッパーと ROS 2 component の両方で共有 |
| [`hybrid_package_msgs/`](samples/hybrid_package_msgs/) | `generate_ros1_compat_headers()` を使ったハイブリッド msg パッケージ |
| [`hybrid_package_pubsub_samples/`](samples/hybrid_package_pubsub_samples/) | Pub / sub パイプライン + intra-process Zero Copy 規約 |

## 単体パッケージとしてのビルド / テスト

catkin ワークスペースに投入する例:

```bash
ln -s /path/to/sq_ros1_rclcpp_compat/sq_ros1_rclcpp_compat <workspace>/src/sq_ros1_rclcpp_compat
catkin build sq_ros1_rclcpp_compat
catkin run_tests sq_ros1_rclcpp_compat
```

[`sq_ros_hybrid_kit`](https://github.com/seqsense/sq_ros_hybrid_kit) の Docker 環境を使えば、本リポジトリの clone・symlink 配置・`make build-ros1 PKGS=sq_ros1_rclcpp_compat` / `make test-ros1` まで自動化されている。

## スコープと制限

- shim はマイグレーションサンプルが実際に使う rclcpp サーフェスのみ (logging, parameter, time, 標準 msg, tf2) をカバーする。**完全な `rclcpp` 再実装ではない** — services、actions、lifecycle nodes、QoS profile、callback group などは意図的に未提供。
- **API の追加は歓迎、API の変更は不可**: 一度公開したシグネチャを変更すると、shim と本物の `rclcpp` の差異がハイブリッドビルドの前提を壊す。
- ハイブリッドパターンの対象ノードは C++ のみ。Python ノードはスコープ外。

## ライセンス

Apache-2.0. [LICENSE](LICENSE) を参照してください。
