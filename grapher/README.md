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

## 4. OSMからグラフ構築
まず、OSMのデータは`Node`、`Way`、`Relation`の３つで構成されます。
`Node`は緯度経度を持つ点です。
`Way`は、`Node`の配列です。この配列を順番にたどると、一つの`線`となります。
`Releation`はここでは用いません。

一方グラフ構造では、ノード(頂点)とエッジ(辺)で構成されます。
各ノードが、接続先のノード情報としてエッジを持つ構造です。

OSMからグラフ構築では、`線`であるOSMの`Way`から、エッジに変換する処理をすることが目的です。
この課題研究では、分かりづらいですが、OSMの`Way`に対応してエッジをウェイと呼んでいます。
プログラムでは、`線`であるOSMの`Way`を`osm_way`、グラフ構造のエッジを`way`としています。

> [!NOTE]
> ここでのグラフ構造は無向グラフです。
> 自己ループや多重辺は元データによっては生じる可能性があります(非単純)。
> 独立したネットワークが存在する可能性があります(非連結)。

グラフ構築の実装を以下に示します。
```cpp
bool invalid_osm_way(const osmium::Way &osm_way) {
  if(!osm_way.tags().has_key("highway")) return true;
  if(osm_way.nodes().size() < 2) return true;
  return false;
}

void Graph::construct() {
  using construct_NodeMemo = std::unordered_set<ID>;

  struct construct_Handler1 : public osmium::handler::Handler {
    Graph &graph;
    construct_NodeMemo &memo;
    construct_Handler1(Graph &graph, construct_NodeMemo &memo): graph(graph), memo(memo) {}
  
    void way(const osmium::Way &osm_way) {
      if(invalid_osm_way(osm_way)) return;
  
      const osmium::WayNodeList &osm_nodes = osm_way.nodes();
      for(int i = 0; i < osm_nodes.size(); i++) {
        memo.insert(osm_nodes[i].ref());
      }
    }
  };
  
  struct construct_Handler2 : public osmium::handler::Handler {
    Graph &graph;
    construct_NodeMemo &memo;
    construct_Handler2(Graph &graph, construct_NodeMemo &memo): graph(graph), memo(memo) {}
  
    void node(const osmium::Node &osm_node) {
      if(memo.find(osm_node.id()) == memo.end()) return;
  
      const auto &loc = osm_node.location();
      graph.put(
        osm_node.id(),
        Node(graph.nc(), osm_node.id(), loc.lon(), loc.lat())
      );
    }
  };
  
  struct construct_Handler3 : public osmium::handler::Handler {
    Graph &graph;
    construct_NodeMemo &memo;
    construct_Handler3(Graph &graph, construct_NodeMemo &memo): graph(graph), memo(memo) {}
  
    void way(const osmium::Way &osm_way) {
      if(invalid_osm_way(osm_way)) return;
  
      const osmium::WayNodeList &osm_nodes = osm_way.nodes();
      for(int i = 1; i < osm_nodes.size(); i++) {
        Node &from = graph.get(osm_nodes[i-1].ref());
        Node &to   = graph.get(osm_nodes[i].ref());
  
        const double length = Node::distance(from, to);
        from.add_way(Way(graph.wc(), osm_way.id(), to.id(), length));
        to.add_way(Way(graph.wc(), osm_way.id(), from.id(), length));
      }
    }
  };

  construct_NodeMemo memo;

  // 1. ウェイのみを読んで、ウェイに存在するノードをmemoに追加する
  osmium::io::Reader r1(file_);
  construct_Handler1 h1(*this, memo);
  osmium::apply(r1, h1);
  r1.close();

  // 2. memoにあるノードの情報を読んで、グラフに追加
  osmium::io::Reader r2(file_);
  construct_Handler2 h2(*this, memo);
  osmium::apply(r2, h2);
  r2.close();

  // 3. もう一度ウェイを読んで、ノードにウェイを追加する
  osmium::io::Reader r3(file_);
  construct_Handler3 h3(*this, memo);
  osmium::apply(r3, h3);
  r3.close();
}
```
上から順番に見ていきましょう。

### (1) 利用するOSMのウェイ、ノードを特定
```cpp
struct construct_Handler1 : public osmium::handler::Handler {
  Graph &graph;
  construct_NodeMemo &memo;
  construct_Handler1(Graph &graph, construct_NodeMemo &memo): graph(graph), memo(memo) {}
  
  void way(const osmium::Way &osm_way) {
    if(invalid_osm_way(osm_way)) return;
  
    const osmium::WayNodeList &osm_nodes = osm_way.nodes();
    for(int i = 0; i < osm_nodes.size(); i++) {
      memo.insert(osm_nodes[i].ref());
    }
  }
};
```
```cpp
// 1. ウェイのみを読んで、ウェイに存在するノードをmemoに追加する
osmium::io::Reader r1(file_);
construct_Handler1 h1(*this, memo);
osmium::apply(r1, h1);
r1.close();
```
`construct_Handler1::way`でOSMデータのすべてのウェイについて処理します。
`invalid_osm_way`で道路データのウェイだけにフィルタします。
`invalid_osm_way`については`(4) 道路データのフィルタ`で説明します。
そして、ウェイの持つすべてのノードを`memo`に追加します。

この`memo`に追加するという遠回りな処理をする理由には、以下の２つがあります。
1. OSMで道路を特定するにはノードではなく、ウェイで判定する必要があること。
1. OSMのウェイはノードの参照のみを持っていて、ノードの実体は別で読む必要があること。

### (2) 利用するノードを読んで、グラフノードを構築
```cpp
struct construct_Handler2 : public osmium::handler::Handler {
  Graph &graph;
  construct_NodeMemo &memo;
  construct_Handler2(Graph &graph, construct_NodeMemo &memo): graph(graph), memo(memo) {}

  void node(const osmium::Node &osm_node) {
    if(memo.find(osm_node.id()) == memo.end()) return;

    const auto &loc = osm_node.location();
    graph.put(
      osm_node.id(),
      Node(graph.nc(), osm_node.id(), loc.lon(), loc.lat())
    );
  }
};
```
```cpp
// 2. memoにあるノードの情報を読んで、グラフに追加
osmium::io::Reader r2(file_);
construct_Handler2 h2(*this, memo);
osmium::apply(r2, h2);
r2.close();
```
`construct_Handler2::node`でOSMデータのすべてのノードについて処理します。
読んだノードが、`(1) 利用するOSMのウェイ、ノードを特定`で`memo`に記録してあるものだったら処理を続けます。
`graph`にノードを構築します。

### (3) 利用するウェイを読んで、グラフエッジを構築
```cpp
struct construct_Handler3 : public osmium::handler::Handler {
  Graph &graph;
  construct_NodeMemo &memo;
  construct_Handler3(Graph &graph, construct_NodeMemo &memo): graph(graph), memo(memo) {}

  void way(const osmium::Way &osm_way) {
    if(invalid_osm_way(osm_way)) return;

    const osmium::WayNodeList &osm_nodes = osm_way.nodes();
    for(int i = 1; i < osm_nodes.size(); i++) {
      Node &from = graph.get(osm_nodes[i-1].ref());
      Node &to   = graph.get(osm_nodes[i].ref());

      const double length = Node::distance(from, to);
      from.add_way(Way(graph.wc(), osm_way.id(), to.id(), length));
      to.add_way(Way(graph.wc(), osm_way.id(), from.id(), length));
    }
  }
};
```
```cpp
// 3. もう一度ウェイを読んで、ノードにウェイを追加する
osmium::io::Reader r3(file_);
construct_Handler3 h3(*this, memo);
osmium::apply(r3, h3);
r3.close();
```
`construct_Handler3::way`でOSMデータのすべてのウェイについて処理します。
`(1) 利用するOSMのウェイ、ノードを特定`と同様に道路データのウェイだけにフィルタします。

次に、OSMのウェイが持っているOSMのノード参照を反復処理します。
OSMのウェイが`線`であることを思い出してください。
ウェイが配列として持つノード参照は隣あうノード参照と接続されている状態です。
つまり、ノード参照`i`はノード参照`i-1`とノード参照`i+1`と接続すればよいのです。

グラフ構造での接続は、あるノードに対して、接続先のノードに向かうエッジを生やすことです。
無向グラフであるため、二つのノードを接続するとき、相互にエッジを生やします。
`add_way`で`(2) 利用するノードを読んで、グラフノードを構築`で構築したノードに対してエッジを追加しています。

> [!NOTE]
> この部分の処理は`(1) 利用するOSMのウェイ、ノードを特定`で行うこともできます。
> ですが、意図を明確にするために、分ける設計を選択しました。
> OSMデータファイルの読み取り回数は増えますが、オーダーは変わりませんから。

### (4) 道路データのフィルタ
```cpp
bool invalid_osm_way(const osmium::Way &osm_way) {
  if(!osm_way.tags().has_key("highway")) return true;
  if(osm_way.nodes().size() < 2) return true;
  return false;
}
```
`(1) 利用するOSMのウェイ、ノードを特定`ででてきた、道路データ抽出について説明します。

まず、OSMのウェイは道路だけでなく、建物、地形などの輪郭の線、川のデータなど様々あります。
`invalid_osm_way`では、経路探索で扱う道路のウェイだけを抽出します。

`if(!osm_way.tags().has_key("highway")) return true;`は単純にOSMのウェイが`highway`というタグを持っているかどうか、という判定です。
これだけでは、歩行者が通れない道路も抽出される可能性があります。
しかし、判定を厳しくすると、利用可能なウェイデータが減ってしまう可能性があります。
OSMのタグは、人間がつけるもので、ルールなどはありません。
そのため、歩道を判定することは難しいのです。

`if(osm_way.nodes().size() < 2) return true;`という処理は、歩行可能かどうかの判定とは関係ありません。
ノードを1つしか持っていないOSMウェイは、単純にウェイデータとして成り立っていないので取り除きます。
この処理がなくても、grapher全体としては正しく動作します。
