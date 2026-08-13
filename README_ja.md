# sq_ros1_rclcpp_compat

English version: [README.md](README.md)

ROS 1 上で ROS 2 の `rclcpp` API を使うための shim ライブラリと、よく使うメッセージパッケージの rclcpp スタイルヘッダ群。

想定ユースケースは **段階的な ROS 1 → ROS 2 移行**: ノードのロジック層を ROS 2 API に書き換えつつ、ROS 1 でもビルドし続けられるようにすること。

[sq_ros_hybrid_kit](https://github.com/seqsense/sq_ros_hybrid_kit) は、本リポジトリの shim を利用して既存の ROS 1 C++ パッケージを ROS 1 (Noetic or ROS One) / ROS 2 (Humble or Jazzy) 両方でビルド可能なハイブリッドパッケージへ変換する AI エージェントと、両環境のビルド・実行用 Docker 環境を提供している。

## ハイブリッド化の設計思想

ハイブリッド C++ パッケージは 3 層で構成する:

- **ROS 1 / ROS 2 共用ロジック層** — `rclcpp` で記述
- **ROS 1 インタフェース層** — `roscpp` で記述
- **ROS 2 インタフェース層** — `rclcpp` で記述

`roscpp` と `rclcpp` のノードインタフェースは差異が大きく共通化に向かないため、インタフェース層は二重に書く。共通化するのはロジック層だけで、その共通化を成立させるのが本パッケージである。すなわち、ロジック層が避けて通れない rclcpp サーフェス (`Logger`, `Clock` / `Time`, メッセージ型, `RCLCPP_*` マクロ, throttle ログなど) の ROS 1 環境向け shim、メッセージ型の変換ラッパーなどを提供する。

ハイブリッド化後にROS 2 環境で十分動作確認が取れ、ROS 1 サポートが不要になった後は、ROS 1 インタフェース層と、`CMakeLists.txt`, `package.xml` の ROS 1 関連部分の記述(本パッケージへの依存を含む)を削除するだけで、純粋な ROS 2 パッケージとすることができる。

設計の典型例は [`samples/hybrid_imu_analyzer/`](samples/hybrid_imu_analyzer/) 配下を参照。共有ロジック [`src/imu_analyzer.cpp`](samples/hybrid_imu_analyzer/src/imu_analyzer.cpp) と、2 つのインタフェースラッパー [`src/ros1_imu_analyzer_node.cpp`](samples/hybrid_imu_analyzer/src/ros1_imu_analyzer_node.cpp) / [`src/ros2_imu_analyzer_node.cpp`](samples/hybrid_imu_analyzer/src/ros2_imu_analyzer_node.cpp) を読み比べると分かりやすい。


## 提供物

### ROS 1 インタフェース層用 helper

[sq_ros1_rclcpp_compat/include/sq_ros1_compat/](sq_ros1_rclcpp_compat/include/sq_ros1_compat/) 配下のヘッダは、ROS 1 インタフェース層から include し、共用ロジック層に受け渡すオブジェクトを生成するためのものである。

| ヘッダ | 提供物 | 用途 |
|-------|------|------|
| `sq_ros1_compat/logger.hpp` | `sq_ros1_compat::get_logger(name)` | 共用ロジック層に渡す `rclcpp::Logger` を生成する |
| `sq_ros1_compat/msg_ptr.hpp` | `sq_ros1_compat::to_std()` / `to_boost()` | メッセージ型の `boost::shared_ptr` <-> `std::shared_ptr` の変換用 |

共用ロジック層のクラスは、`rclcpp::Logger` をコンストラクタなどで受け取れるようにすることを推奨する。ROS 2 でビルドされた場合は本来の `rclcpp::Logger`、ROS 1 でビルドされた場合は `sq_ros1_compat::get_logger(name)` によって作成された shim オブジェクトを介してログを出力することができる。

ROS メッセージについて、ROS 1 の generator が生成するポインタ型 (`Ptr` / `ConstPtr`) は `boost::shared_ptr` ベースであり、ROS 2 のもの (`SharedPtr` / `ConstSharedPtr`) は `std::shared_ptr` ベースであるため、単純な置き換えはできない。本パッケージを利用する上では、共用ロジック層ではメッセージ型のポインタを `std::shared_ptr<T>` もしくは `std::shared_ptr<const T>` とし、ROS 1 インタフェース層側では `sq_ros1_compat::to_std()` を介して `boost::shared_ptr` から変換することを推奨する。この関数は aliasing-constructor を利用することで、deep copy なしにポインタ型を変換することができる。

### ROS 1 / ROS 2 共用ロジック層用 helper

前述の通り、共用ロジック層は ROS 2 (rclcpp) の記法で記述する。それを ROS 1 環境からビルドするため、以下を提供する。

#### `rclcpp` shim

共用ロジック層が使う rclcpp API を ROS 1 上で提供する。ヘッダは [sq_ros1_rclcpp_compat/include/rclcpp/](sq_ros1_rclcpp_compat/include/rclcpp/) 配下。

| 提供物 | 説明 |
|-------|------|
| `rclcpp::Logger` | バックエンドは `spdlog` |
| `RCLCPP_DEBUG/INFO/WARN/ERROR` / 各 `_THROTTLE` 版 | throttle は `std::chrono::steady_clock` ベース |
| `rclcpp::Clock` / `Time` / `Duration` | 時刻・時間 (時計種別ごとのバックエンドは下記注記) |
| `rclcpp::Node` | パラメータ API と `get_logger()` / `now()` / `get_name()` などの最小限の helper。pub/sub など ROS 2 のノード機能は持たない |

`rclcpp::Clock` のバックエンドは時計種別で異なる: `RCL_ROS_TIME` は `ros::Time`、`RCL_SYSTEM_TIME` / `RCL_STEADY_TIME` は `ros::WallTime`。ただし `ros::WallTime` は Wall Clock であるため、`RCL_STEADY_TIME` を指定しても単調増加は保証されない点に注意。

#### ROS 1 / ROS 2 のメッセージヘッダ整合

ROS 1 と ROS 2 では、メッセージの include パスと型名が以下のように変わる。

| | include | 型名 |
|-------|------|------|
| ROS 1 | `#include "std_msgs/String.h"` | `std_msgs::String`|
| ROS 2 | `#include "std_msgs/msg/string.hpp"` | `std_msgs::msg::String`|

本パッケージでは、ROS 2 記法を ROS 1 から使うためのラッパーヘッダを生成できる。すなわち、`<pkg>/msg/<snake_case>.hpp` という名前のヘッダを生成し、その中で ROS 1 の元ヘッダを include した上で、`<pkg>::msg::<CamelCase>` を ROS 1 型への using-alias として再公開する。サービスも同様に `<pkg>/srv/<snake_case>.hpp` として生成する。ROS 1 のサービスクラスは元々 `Request` / `Response` のメンバ typedef を持つため、同じ alias 1 行で ROS 2 記法の `<pkg>::srv::<CamelCase>::Request` がそのまま通る。

ROS 1 の中核インターフェースパッケージについては、本パッケージ内で自動的に ROS 1 用ラッパーヘッダを作成する。対象は `std_msgs`, `std_srvs`, `common_msgs` 一式 (`actionlib_msgs`, `diagnostic_msgs`, `geometry_msgs`, `nav_msgs`, `sensor_msgs`, `shape_msgs`, `stereo_msgs`, `trajectory_msgs`, `visualization_msgs`), `tf2_msgs`。独自インターフェースパッケージでは `CMakeLists.txt` から `generate_ros1_compat_headers()` を呼び出すことで、ROS 1 用 ラッパーヘッダを作成することができる。具体的な例は [samples/hybrid_package_msgs/](samples/hybrid_package_msgs/) を参照。

アクションについては意図的に alias を生成していない。ROS 1 は `Foo.action` を 7 つのフラットなメッセージ (`FooAction`, `FooGoal`, ...) に展開するが、ROS 2 は同じものを `Goal` / `Result` / `Feedback` をネストした単一のアクション型として表現する。すなわち alias 先となる `nav_msgs::msg::GetMapAction` は ROS 2 に存在しないため、これらの名前は生成対象から除外している。

なお、ラッパーヘッダは alias を与えるだけで、alias 先のパッケージを連れてくるわけではない。利用するインターフェースパッケージは、本パッケージを使わない場合と同様に各パッケージ側で宣言する必要がある。`sensor_msgs/PointCloud2.h` が include できない環境では、`sensor_msgs` のラッパーヘッダがあっても意味を持たない。

なお、生成するのは型の using-alias のみで、`SharedPtr` / `ConstSharedPtr` などのポインタ型メンバは定義していない。using-alias は既存の ROS 1 型に別名を与えるだけでメンバを追加できず、仮に別型として定義すると型の同一性が崩れて pub/sub や message-traits の特殊化と噛み合わなくなるためである。前述の通り、共用ロジック層ではメッセージのポインタを `std::shared_ptr<T>` もしくは `std::shared_ptr<const T>` で宣言し、ROS 1 インタフェース層で `sq_ros1_compat::to_std()` を介して変換することを推奨する。

#### ROS 1 向けのインターフェース定義の書き換え (`preprocess_ros1_interfaces()`)

インタフェース定義についても ROS 2 記法で記述し、ROS 1 でビルドする際に本節で紹介する `preprocess_ros1_interfaces()` を用いてキーワード置換を行うことを推奨する。ROS 1 と ROS 2 で記法が異なり、置換が必要な組み込み型は以下の 3 つだけである。

| ROS 1 記法 | ROS 2 記法 |
|-----------|-----------|
| `Header` | `std_msgs/Header` |
| `time` | `builtin_interfaces/Time` |
| `duration` | `builtin_interfaces/Duration` |

これ以外の組み込み型 (`bool` / `int8` / `uint8` / `float64` / `string` や配列、`byte` など) と、`pkg/Type` 形式のパッケージ修飾型は ROS 1 / ROS 2 で記法が同一なので置換不要である。

上記のうち `std_msgs/Header` は ROS 1 / ROS 2 のいずれでもビルドが通るため、ソース側で `Header` を `std_msgs/Header` に置き換えておくだけでよい。

残りの `builtin_interfaces/Time` / `builtin_interfaces/Duration` は ROS 1 の生成器が解釈できないため、ROS 1 ビルド時にそれぞれ `time` / `duration` へ書き戻す CMake 関数 `preprocess_ros1_interfaces()` を用意した。以下のようにして利用することができる。

```cmake
preprocess_ros1_interfaces(SUBDIR msg FILES ${MSG_FILES} OUTPUT_DIR_VAR ROS1_MSG_DIR)
add_message_files(DIRECTORY ${ROS1_MSG_DIR} FILES ${MSG_FILES})
```

| キーワード引数 | 内容 |
|--------------|------|
| `SUBDIR` | インタフェースファイルが格納されたサブディレクトリ名 (`msg` / `srv` / `action` のいずれか) |
| `FILES` | インタフェースファイル名のリスト |
| `OUTPUT_DIR_VAR` | 置換後のファイルを格納したディレクトリのパスを受け取る変数の名前 (関数がこの名前の変数を呼び出し元スコープに定義する) |

`add_message_files` (同様に `add_service_files` / `add_action_files`) に、ROS 1 記法へ置換した後のディレクトリ (`OUTPUT_DIR_VAR` で受け取った変数) とファイル名を渡すことでビルドが通る仕組みである。

#### `*.hpp` ヘッダへのラッパー

以下について、ROS 1 では `*.h` であったヘッダが ROS 2 では `*.hpp` にリネームされているため、本パッケージでは `*.h` に転送するだけの単純なラッパーを提供している。

- [tf2_eigen/tf2_eigen.hpp](sq_ros1_rclcpp_compat/include/tf2_eigen/tf2_eigen.hpp)
- [tf2_geometry_msgs/tf2_geometry_msgs.hpp](sq_ros1_rclcpp_compat/include/tf2_geometry_msgs/tf2_geometry_msgs.hpp)
- [tf2_sensor_msgs/tf2_sensor_msgs.hpp](sq_ros1_rclcpp_compat/include/tf2_sensor_msgs/tf2_sensor_msgs.hpp)
- [sensor_msgs/point_cloud2_iterator.hpp](sq_ros1_rclcpp_compat/include/sensor_msgs/point_cloud2_iterator.hpp)

### `sq_ros1_rclcpp_compat_gtest_main`

ROS 1 の `catkin_add_gtest` は gtest 標準の `gtest_main` をリンクしないため、本来はテストごとに `main()` を書く必要がある。本パッケージは `main()` を提供し、加えて `ros::Time::init()` を呼ぶ static ライブラリを同梱する。これによりテストコードは追加の boilerplate 無しで `rclcpp::Clock(RCL_ROS_TIME).now()` を使える。このライブラリは `CATKIN_ENABLE_TESTING` が有効なとき (テストビルド時) にのみビルド・export されるが、その際は `${catkin_LIBRARIES}` 経由で export されるため、本パッケージに依存するテストターゲットは transitive にリンクされる (`target_link_libraries(<test> sq_ros1_rclcpp_compat_gtest_main)` を明示する必要はない)。

ROS 2 では `ament_add_gtest` が `gtest_main` を提供するため、上記のような仕組みは必要ない。

利用例: [`samples/hybrid_imu_analyzer/test/`](samples/hybrid_imu_analyzer/test/) と [`CMakeLists.txt`](samples/hybrid_imu_analyzer/CMakeLists.txt) の ROS 1 ブランチを参照。

## ディレクトリ構成

```
sq_ros1_rclcpp_compat/
├── sq_ros1_rclcpp_compat/   # shim パッケージ本体 (catkin / ament_cmake)
│   ├── include/             # 公開ヘッダ
│   ├── cmake/               # generate_ros1_compat_headers / preprocess_ros1_interfaces ヘルパ
│   ├── scripts/             # 組込対象外の msg パッケージ用に単独の compat パッケージを生成するスクリプト
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
#include <std_msgs/msg/string.hpp>      // 自動生成されたラッパーヘッダ

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
