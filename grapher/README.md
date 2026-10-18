# grapher
OSMのデータから指標情報を持つグラフ構造`bg`を構築するC++ライブラリです。
経路探索処理は含まれません。

## 1. 指標とは
指標(Indicator)とは、Node、Wayに付属する特徴データです。
指標は数値であり、Node、Wayそれぞれに複数付加することができます。

## 2. bg作成方法
`bg`を作成するサンプルコードを以下に示します。

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

Nodeの指標は`gp::NodeEvaluator`、Wayの指標は`gp::WayEvaluator`を用いてい定義します。
そして、それらのインスタンスを`gp::NodeContext`、`gp::WayContext`に登録します。
最後に、`gp::Graph`を作成して、必要なら`gp::Graph::write`で保存できます。

> [!IMPORTANT]
> 地理的特徴の指標値はWayに付加してください。
> Nodeでは、滑らかな曲線の道などで、複数のNodeが同じ地理的特徴を持つことで、経路探索時の合計コストが重複して大きくなってしまうからです。
> Wayでは、beltgraph側で長さスケーリングが行われるため、断片的なWayが複数あってもこれを回避できます。

> [!WARNING]
> Node、Wayの数制限はbeltgraphの実装により$ 2^{32}-1 $です。
> これより大きくする必要があればbeltgraphの実装を変更します。
> その場合、beltgraph全体的なメモリ使用量が増加します。

### gp::NodeEvaluator
Node指標定義でオーバライドするメンバ関数を以下に示します。

|メンバ関数|説明|
|-|-|
|gp::IndicatorName name() const|指標名|
|gp::IndicatorDescription description() const|指標説明文|
|gp::Indicator evaluate_from_osm(const osmium::Node &osm_node)|OSMのノードデータを用いた指標値計算|

### gp::WayEvaluator
Way指標定義でオーバライドするメンバ関数を以下に示します。

|メンバ関数|説明|
|-|-|
|gp::IndicatorName name() const|指標名|
|gp::IndicatorDescription description() const|指標説明文|
|gp::Indicator evaluate_from_osm(const osmium::Way &osm_Way)|OSMのウェイデータを用いた指標値計算|

## 3. 指標計算方法
`2. 指標作成方法`で説明した指標値計算は、正確には指標となる前の評価値($ \bm{E} $)の計算です。
指標値($ \bm{indicator} $)はこれをさらに加工します。その過程を以下に示します。

$$
\bm{E}_i :=
\left\{ \begin{matrix}
\bm{E}_i & (\bm{E}_i \ge 0) \\
0 & (\bm{E}_i < 0) \\
\end{matrix} \right\}
$$
$$
\bm{indicator}_i = 
\left\{ \begin{matrix}
\displaystyle \frac{\bm{E}_i - \bm{Emin}_i}{\bm{Emax}_i - \bm{Emin}_i} & (\bm{Emax}_i - \bm{Emin}_i \ne 0) \\
1 & (\bm{Emax}_i - \bm{Emin}_i = 0) \\
\end{matrix} \right\}
$$

評価値、指標値は0以上に強制します。正規化で0除算が発生する場合は指標値を1とします。

> [!NOTE]
> 正規化で0除算が発生する場合は、その指標が機能していない状態です。
