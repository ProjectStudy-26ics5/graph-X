# grapher
OSMのデータからbgを構築するC++ライブラリです。

## 1. 利用方法
`grapher.hpp`をインクルードし、CMakeから`PS::grapher`をリンクすると、すべての機能が使えるようになります。
名前空間`gp::`を用います。

bgを作成するサンプルコードを以下に示します。部分的な説明を続く章で説明します。
```cpp
#include <grapher.hpp>

class TrafficSignal : public gp::NodeEvaluator {
  gp::IndicatorName name() const override {
    return "traffic_signal";
  }
  gp::IndicatorDescription description() const override {
    return "Indicates a traffic signal: 1 when present, otherwise 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Node &osm_node) override {
    return osm_node.tags().has_tag("highway", "traffic_signals") ? 1 : 0;
  }
};

class Surface : public gp::WayEvaluator {
  gp::IndicatorName name() const override {
    return "surface";
  }
  gp::IndicatorDescription description() const override {
    return "Rates basic surfaces: paved 1.0, unpaved 0.3, and other values 0.5.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) override {
    if (osm_way.tags().has_tag("surface", "paved")) return 1.0;
    if (osm_way.tags().has_tag("surface", "unpaved")) return 0.3;
    return 0.5;
  }
};

int main(void) {
  TrafficSignal trafficSignal;
  Surface surface;

  // 指標定義インスタンスの登録
  gp::NodeContext nc({&trafficSignal});
  gp::WayContext wc({&surface});

  // bg作成
  gp::Graph graph("osm/minato.osm.pbf", nc, wc);
  graph.write("bg/minato"); // => bg/minato.bg
  return 0;
}
```

## 2. 指標定義
```cpp
class TrafficSignal : public gp::NodeEvaluator {
  gp::IndicatorName name() const override {
    return "traffic_signal";
  }
  gp::IndicatorDescription description() const override {
    return "Indicates a traffic signal: 1 when present, otherwise 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Node &osm_node) override {
    return osm_node.tags().has_tag("highway", "traffic_signals") ? 1 : 0;
  }
};
```
ノードの指標は`gp::NodeEvaluator`を継承して定義します。
Node指標定義でオーバライドするメンバ関数を以下に示します。

|メンバ関数|説明|戻り値|
|-|-|-|
|gp::IndicatorName name() const|指標名|文字列|
|gp::IndicatorDescription description() const|指標説明文|文字列|
|gp::Indicator evaluate_from_osm(const osmium::Node &osm_node)|OSMのノードデータを用いた、このノードの評価|0以上の実数|

```cpp
class Surface : public gp::WayEvaluator {
  gp::IndicatorName name() const override {
    return "surface";
  }
  gp::IndicatorDescription description() const override {
    return "Rates basic surfaces: paved 1.0, unpaved 0.3, and other values 0.5.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) override {
    if (osm_way.tags().has_tag("surface", "paved")) return 1.0;
    if (osm_way.tags().has_tag("surface", "unpaved")) return 0.3;
    return 0.5;
  }
};
```
ウェイの指標は`gp::WayEvaluator`を継承して定義します。
Way指標定義でオーバライドするメンバ関数を以下に示します。
|メンバ関数|説明|戻り値|
|-|-|-|
|gp::IndicatorName name() const|指標名|文字列|
|gp::IndicatorDescription description() const|指標説明文|文字列|
|gp::Indicator evaluate_from_osm(const osmium::Way &osm_Way)|OSMのウェイデータを用いた、このウェイの評価|0以上の実数|

> [!IMPORTANT]
> 地理的特徴の指標値はウェイに付加してください。
> ノードでは、滑らかな曲線の道などで、複数のノードが同じ地理的特徴を持つことで、経路探索時の合計コストが重複して大きくなってしまうからです。
> ウェイでは、beltgraph側で長さスケーリングが行われるため、断片的なウェイが複数あってもこれを回避できます。

## 3. bg作成
```cpp
TrafficSignal trafficSignal;
Surface surface;

// 指標定義インスタンスの登録
gp::NodeContext nc({&trafficSignal});
gp::WayContext wc({&surface});
```
指標定義のインスタンスの参照を渡して、`gp::NodeContext`、`gp::WayContext`を作成します。
複数の指標定義をbraced-init-listを用いて指定できます。

```cpp
// bg作成
gp::Graph graph("osm/minato.osm.pbf", nc, wc);
graph.write("bg/minato"); // => bg/minato.bg
```
入力するOSMデータファイルパスと、`gp::NodeContext`、`gp::WayContext`を渡して、`gp::Graph`を作成します。
`gp::Graph::write`でファイル名を指定して、bgとして保存できます。

> [!WARNING]
> bgのノード、ウェイの数制限はbeltgraphの実装により$ 2^{32}-1 $です。
> これより大きくする必要があればbeltgraphの実装を変更します。
> その場合、beltgraph全体的なメモリ使用量が増加します。
