# ProjectStudy-26ics5 Graph

2026年度 東京科学大学付属科学技術高等学校 課題研究 情報システム分野 5班<br>
課題研究の２つのコアライブラリを提供します。
* grapher
* beltgraph

## 1. 利用方法
このプロジェクトを利用するには、以下のツールが事前にインストールされている必要があります。
* C/C++コンパイラ
* CMake
* vcpkg

インストールは以下のコマンドでできます。

> [!CAUTION]
> インストールの準備がまだ完全にできていません。

```
cmake --preset release
cmake --build --preset release
cmake --install build
```

## 2. OpenStreetMap (OSM)
このプロジェクトでは地図データとして<a href="https://openstreetmap.org/">OpenStreetMap</a>のデータを用います。
また、C++ライブラリ<a href="https://osmcode.org/libosmium/">libosmium</a>を用いてOpenStreetMapのデータを読み取ります。

## 3. grapher
OSMのデータから指標情報を持つグラフ構造`bg`を構築するC++ライブラリです。
詳細は`grapher/README.md`を参照してください。

## 4. beltgraph
grapherで作成した`bg`を読み、経路探索処理を提供するCライブラリです。
詳細は`beltgraph/README.md`を参照してください。

## 5. テストの実行
テストは`tests/`にあります。
実行コマンドは`Taskfile.yml`を参照してください。
go-taskを持っていたら、そのまま実行できます。

## 6. bgの構造
beltgraphライブラリ専用の指標情報を持つグラフ構造です。
一般的なグラフ構造におけるノードをNode、エッジをWayとしています。
`bg`は非圧縮のバイナリファイルとして保存され、グラフはCSR(Compressed Sparse Row)形式のような構造を持ちます。
構造を以下に示します。

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
          <li>Reverse Way idx</li>
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
