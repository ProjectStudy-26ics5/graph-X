# ProjectStudy-26ics5 Graph

このプロジェクトは以下２つのC++ライブラリを提供します。
* `grapher`
* `beltgraph`

## 1. ツール
使用しているツールを以下に示します。
* vcpkg
* cmake

## 2. grapher
OSMのデータから指標情報を持つグラフ構造を構築するライブラリです。
経路探索に用いる処理は一切含まれません。
結果は`beltgraph`ライブラリ専用の`.bg`形式となります。

### bgファイル作成方法
作成方法を以下に示します。
ノードの指標は`gp::NodeEvaluator`、ウェイの指標は`gp::WayEvaluator`を用いてい定義します。

```cpp
#include <grapher.hpp>

struct TrafficSignal : public gp::NodeEvaluator {
  gp::IndicatorName name() const {
    return "traffic_signal";
  }
  gp::IndicatorDescription description() const {
    return "Indicates a traffic signal: 1 when present, otherwise 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Node &osm_node) {
    return osm_node.tags().has_tag("highway", "traffic_signals") ? 1 : 0;
  }
};

struct Surface : public gp::WayEvaluator {
  gp::IndicatorName name() const {
    return "surface";
  }
  gp::IndicatorDescription description() const {
    return "Rates basic surfaces: paved 1.0, unpaved 0.3, and other values 0.5.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) {
    if (osm_way.tags().has_tag("surface", "paved")) return 1.0;
    if (osm_way.tags().has_tag("surface", "unpaved")) return 0.3;
    return 0.5;
  }
};

int main(void) {
  TrafficSignal trafficSignal;
  Surface surface;

  gp::NodeContext nc({&trafficSignal});
  gp::WayContext wc({&surface});

  gp::Graph graph("osm/minato.osm.pbf", nc, wc);
  graph.write("out/test");
  return 0;
}
```

## 3. beltgraph
`grapher`で作成した`.bg`を読み、経路探索処理を提供します。
