# ProjectStudy-26ics5 Graph

このプロジェクトは以下２つのC++ライブラリを提供します。
* grapher
* beltgraph

## 1. 前提条件
このプロジェクトを動かすには、以下のツールが事前にインストールされている必要があります。
* C/C++コンパイラ
* CMake
* Ninja
* vcpkg
* Task

## 2. OpenStreetMap (OSM)
このプロジェクトでは地図データとして<a href="https://openstreetmap.org/">OpenStreetMap</a>のデータを用います。
また、C++ライブラリ<a href="https://osmcode.org/libosmium/">libosmium</a>を用いてOpenStreetMapのデータを読み取ります。

## 3. grapher
OSMのデータから指標情報を持つグラフ構造を構築するライブラリです。
経路探索処理は含まれません。
結果はbeltgraphライブラリ専用の`.bg`形式となります。

### bg作成サンプル
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
  graph.write("bg/minato"); // => bg/minato.bg
  return 0;
}
```

### テスト実行
`tests/converter.cpp`を以下のコマンドで実行できます。
```
task run-converter
```

## 4. beltgraph
grapherで作成した`.bg`を読み、経路探索処理を提供します。

> [!CAUTION]
> まだ、この部分は完成していません。

## 5. ライブラリの利用方法
自分の環境にインストールは以下のコマンドでできます。
```
task install
```
もしくは、
```
task install PREFIX="path/to/your/install/dir"
```

> [!CAUTION]
> grapherのインストールの準備ができていません。

CMakeから以下のようにして、beltgraphを利用できます。
```
find_package(beltgraph CONFIG REQUIRED)

add_executable(sample)

target_sources(
  sample
  PRIVATE
  sample.c
)

target_link_libraries(
  sample
  PRIVATE
  PS::beltgraph
)
```

## 6. bg形式の構造
指標情報を持つグラフ構造です。
一般的なグラフ構造におけるノードをNode、エッジをWayとしています。
`.bg`は非圧縮のバイナリファイルとして保存され、グラフはCSR(Compressed Sparse Row)形式のような構造を持ちます。
`.bg`形式の構造を以下に示します。

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
      <li>NodeIndicatorNameOffsetArray
        <ul>
          <li>NodeIndicatorNameOffset 1</li>
          <li>NodeIndicatorNameOffset 2</li>
          <li>...</li>
          <li>NodeIndicatorNameOffset k</li>
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
      <li>NodeIndicatorDescriptionOffsetArray
        <ul>
          <li>NodeIndicatorDescriptionOffset 1</li>
          <li>NodeIndicatorDescriptionOffset 2</li>
          <li>...</li>
          <li>NodeIndicatorDescriptionOffset k</li>
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
      <li>WayIndicatorNameOffsetArray
        <ul>
          <li>WayIndicatorNameOffset 1</li>
          <li>WayIndicatorNameOffset 2</li>
          <li>...</li>
          <li>WayIndicatorNameOffset l</li>
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
      <li>WayIndicatorDescriptionOffsetArray
        <ul>
          <li>WayIndicatorDescriptionOffset 1</li>
          <li>WayIndicatorDescriptionOffset 2</li>
          <li>...</li>
          <li>WayIndicatorDescriptionOffset l</li>
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
