# sq_ros1_rclcpp_compat

English version: [README.md](README.md)

ROS 1 上で ROS 2 の `rclcpp` API を使うための shim ライブラリと、よく使うメッセージパッケージの rclcpp スタイルヘッダ群。同じ C++ ソースを ROS 1 (Noetic) と ROS 2 (Jazzy / Humble) の両方でビルドできる (ROS 2 上では意図的に no-op)。

想定ユースケースは **段階的な ROS 1 → ROS 2 移行**: ノードのロジック層を ROS 2 API に書き換えつつ、ROS 1 でもビルドし続けられるようにすること。

[sq_ros_hybrid_kit](https://github.com/seqsense/sq_ros_hybrid_kit) は、本リポジトリの shim を利用して既存の ROS 1 パッケージを ROS 1 / ROS 2 両対応のハイブリッドパッケージへ変換する AI エージェントと、両環境のビルド・実行用 Docker 環境を提供している。

## ハイブリッド化の設計思想

ハイブリッド C++ パッケージは 3 層で構成する:

- **ロジック層** — `rclcpp` API で書かれた純 C++。`.cpp` / `.hpp` 各 1 セットを両ビルド間で共有し、gtest も同様に共通化できる。
- **ROS 1 インタフェース層** — `roscpp` で記述。
- **ROS 2 インタフェース層** — `rclcpp` で記述。

`roscpp` と `rclcpp` のノードインタフェースは差異が大きく共通化に向かないため、**インタフェース層は意図的に二重に書く**。共通化するのはロジック層だけで、その共通化を成立させるのが本 shim — ロジック層が避けて通れない rclcpp サーフェス (`Logger`, `Clock` / `Time`, メッセージ型, `RCLCPP_*` マクロ, throttle ログなど) を `roscpp` の上に提供する。

多くの場合、複雑なロジック部分を切り出して単体テストで動作を保証できれば、既存の ROS 1 インタフェース部分を ROS 2 インタフェースへマイグレーションする作業は比較的容易である。

設計の典型例は [`samples/hybrid_imu_analyzer/`](samples/hybrid_imu_analyzer/) 配下を参照。共有ロジック [`src/imu_analyzer.cpp`](samples/hybrid_imu_analyzer/src/imu_analyzer.cpp) と、2 つのインタフェースラッパ [`src/ros1_imu_analyzer_node.cpp`](samples/hybrid_imu_analyzer/src/ros1_imu_analyzer_node.cpp) / [`src/ros2_imu_analyzer_node.cpp`](samples/hybrid_imu_analyzer/src/ros2_imu_analyzer_node.cpp) を読み比べると分かりやすい。


## 提供物

### `rclcpp` shim

`rclcpp::Logger`、`rclcpp::Clock` / `Time` / `Duration`、`rclcpp::Node` (パラメータ API)、および `RCLCPP_DEBUG/INFO/WARN/ERROR` と各 `_THROTTLE` 版。ヘッダは [sq_ros1_rclcpp_compat/include/rclcpp/](sq_ros1_rclcpp_compat/include/rclcpp/) 配下。ログ出力は `spdlog`、throttle は `std::chrono::steady_clock` ベース。

### ROS 1 / ROS 2 のメッセージヘッダ整合

ROS 2 では `#include <std_msgs/msg/string.hpp>` で型名 `std_msgs::msg::String` を扱う。ROS 1 では `#include <std_msgs/String.h>` で型名 `std_msgs::String`。同じソースを両ビルドで通すため、本パッケージは ROS 1 ビルド時に compat ヘッダを `<pkg>/msg/<snake_case>.hpp` に自動生成する。生成されたヘッダは ROS 1 の元ヘッダを include した上で、`<pkg>::msg::<CamelCase>` を ROS 1 型を継承した struct として再定義し、`SharedPtr` / `ConstSharedPtr` を ROS 1 の `Ptr` / `ConstPtr` の別名として用意する。

標準パッケージ 6 つ (`std_msgs`, `geometry_msgs`, `nav_msgs`, `sensor_msgs`, `visualization_msgs`, `diagnostic_msgs`) は組込で扱う。独自 msg パッケージは `CMakeLists.txt` から `generate_ros1_compat_headers()` を呼び出して登録する — レシピは [`samples/hybrid_package_msgs/`](samples/hybrid_package_msgs/) を参照。

### `tf2_*` の ROS 2 風ヘッダ

[tf2_eigen/](sq_ros1_rclcpp_compat/include/tf2_eigen/)、[tf2_geometry_msgs/](sq_ros1_rclcpp_compat/include/tf2_geometry_msgs/)、[tf2_sensor_msgs/](sq_ros1_rclcpp_compat/include/tf2_sensor_msgs/) — ROS 1 上で ROS 2 のインクルードレイアウトをそのまま使えるようにする。

### `sensor_msgs::PointCloud2Iterator`

ROS 2 のイテレータをヘッダオンリで移植したもの。[sq_ros1_rclcpp_compat/include/sensor_msgs/](sq_ros1_rclcpp_compat/include/sensor_msgs/) 配下。

### `sq_ros1_rclcpp_compat_gtest_main`

ROS 1 の `catkin_add_gtest` は gtest 標準の `gtest_main` をリンクしないため、本来はテストごとに `main()` を書く必要がある。本パッケージは `main()` を提供し、加えて `ros::Time::init()` を呼ぶ static ライブラリを同梱する — これによりテストコードは追加の boilerplate 無しで `rclcpp::Clock(RCL_ROS_TIME).now()` を使える。`${catkin_LIBRARIES}` 経由で export されるため、本パッケージに依存するテストターゲットは transitive にリンクされる (`target_link_libraries(<test> sq_ros1_rclcpp_compat_gtest_main)` を明示する必要はない)。ROS 2 では `ament_add_gtest` が `gtest_main` を提供するため、本ライブラリは使われない。

利用例: [`samples/hybrid_imu_analyzer/test/`](samples/hybrid_imu_analyzer/test/) と [`CMakeLists.txt`](samples/hybrid_imu_analyzer/CMakeLists.txt) の ROS 1 ブランチを参照。

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

ハイブリッドパターンの対象ノードは C++ のみ。Python など他言語はスコープ外。

## ライセンス

Apache-2.0. [LICENSE](LICENSE) を参照してください。
