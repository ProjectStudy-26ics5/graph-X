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

## 4. beltgraph形式図
<ul>
  <li>Header
    <ul>
      <li>NodeContext position</li>
      <li>NodeContext dimension (= k)</li>
      <li>NodeIndicatorNameArray total byte count</li>
      <li>NodeIndicatorDescriptionArray total byte count</li>
      <li>WayContext position</li>
      <li>WayContext dimension (= l)</li>
      <li>WayIndicatorNameArray total byte count</li>
      <li>WayIndicatorDescriptionArray total byte count</li>
      <li>NodeArray position</li>
      <li>Node count (= m)</li>
      <li>Node byte count</li>
      <li>WayArray position</li>
      <li>Way count (= n)</li>
      <li>Way byte count</li>
    </ul>
  </li>

  <li>NodeContext
    <ul>
      <li>NodeIndicatorNameArray
        <ul>
          <li>NodeIndicatorName 1</li>
          <li>NodeIndicatorName 2</li>
          <li>...</li>
          <li>NodeIndicatorName k
            <ul>
              <li>String</li>
              <li>Null</li>
            </ul>
          </li>
        </ul>
      </li>
      <li>NodeIndicatorDescriptionArray
        <ul>
          <li>NodeIndicatorDescription 1</li>
          <li>NodeIndicatorDescription 2</li>
          <li>...</li>
          <li>NodeIndicatorDescription k
            <ul>
              <li>String</li>
              <li>Null</li>
            </ul>
          </li>
        </ul>
      </li>
    </ul>
  </li>

  <li>WayContext
    <ul>
      <li>WayIndicatorNameArray
        <ul>
          <li>WayIndicatorName 1</li>
          <li>WayIndicatorName 2</li>
          <li>...</li>
          <li>WayIndicatorName l
            <ul>
              <li>String</li>
              <li>Null</li>
            </ul>
          </li>
        </ul>
      </li>
      <li>WayIndicatorDescriptionArray
        <ul>
          <li>WayIndicatorDescription 1</li>
          <li>WayIndicatorDescription 2</li>
          <li>...</li>
          <li>WayIndicatorDescription l
            <ul>
              <li>String</li>
              <li>Null</li>
            </ul>
          </li>
        </ul>
      </li>
    </ul>
  </li>

  <li>NodeArray
    <ul>
      <li>Node 1</li>
      <li>Node 2</li>
      <li>...</li>
      <li>Node m
        <ul>
          <li>Way start idx</li>
          <li>Way end idx</li>
          <li>Longitude</li>
          <li>Latitude</li>
          <li>NodeIndicatorArray
            <ul>
              <li>NodeIndicator 1</li>
              <li>NodeIndicator 2</li>
              <li>NodeIndicator ...</li>
              <li>NodeIndicator k</li>
            </ul>
          </li>
        </ul>
      </li>
    </ul>
  </li>

  <li>WayArray
    <ul>
      <li>Way 1</li>
      <li>Way 2</li>
      <li>...</li>
      <li>Way n
        <ul>
          <li>Node connect idx</li>
          <li>Length</li>
          <li>WayIndicatorArray
            <ul>
              <li>WayIndicator 1</li>
              <li>WayIndicator 2</li>
              <li>WayIndicator ...</li>
              <li>WayIndicator l</li>
            </ul>
          </li>
        </ul>
      </li>
    </ul>
  </li>
</ul>
