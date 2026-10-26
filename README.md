# ProjectStudy-26ics5 Graph
2026年度 東京科学大学付属科学技術高等学校 課題研究 情報システム分野 5班<br>
課題研究の実装における重要な概念と、以下2つのコアライブラリを提供します。
* grapher
* beltgraph

## 1. OpenStreetMap (OSM)
この課題研究では地図データとして<a href="https://openstreetmap.org/">OpenStreetMap</a>のデータを用います。
また、C++ライブラリ<a href="https://osmcode.org/libosmium/">libosmium</a>を用いてOpenStreetMapのデータを読み取ります。

## 2. bg
bg (**b**elt**g**raph) は、この課題研究専用のグラフ構造です。
bgでは、一般的なグラフ構造における頂点やノードを *ノード(Node)* 、辺やエッジを *ウェイ(Way)* と呼びます。
そして、ノード、ウェイそれぞれが *指標* と呼ばれるデータを持つ特殊構造です。
指標(Indicator)とは、ノード、ウェイに付属する特徴データです。
指標は数値であり、ノード、ウェイそれぞれに複数付加することができます。

> [!CAUTION]
> ### bgでの距離の扱い
> grapher、beltgraph両方で距離には緯度経度の角度の距離を用いています。
> これは正しい距離計算ではないです。
> より広い地図を用いる場合や、緯度、経度の符号が変化する場合に問題になります。

## 3. grapher
OSMのデータから指標情報を持つグラフ構造bgを構築するC++ライブラリです。
C++を用いる理由は、beltgraphがC言語であるからです。
利用方法は`grapher/README.md`を参照してください。

## 4. beltgraph
grapherで作成した`bg`を読み、経路探索処理を提供するCライブラリです。
C言語を用いる理由は、マイコンから使いやすくするためです。
利用方法は`beltgraph/README.md`を参照してください。

## 5. 利用方法
> [!CAUTION]
> インストールの準備がまだ完全にできていません。
> 現状はディレクトリをコピーしてください。

このプロジェクトを利用するには、以下のツールが事前にインストールされている必要があります。
* C/C++コンパイラ
* CMake
* vcpkg

インストールは以下のコマンドでできます。

```
cmake --preset release
cmake --build --preset release
cmake --install build
```

## 6. テスト・実験の実行
テストは`tests/`にあります。実験は`experiments/`にあります。

実行コマンドは`Taskfile.yml`を参照してください。
go-taskを持っていたら、そのまま実行できます。
`test-`から始まるタスクがテスト、`e-`から始まるタスクが実験です。

## 7. 付録
grapherとbeltgraphの実装の解説、工夫点を`appendix/*.md`に書いています。
