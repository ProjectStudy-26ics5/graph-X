# beltgraph
grapherで作成した`bg`を読み、経路探索処理を提供するCライブラリです。

## 1. 表記方法
このREADMEでは、構造体名を`<T>`として、`bg<T>Create`のような表記をします。
例えば、構造体名が`Header`なら、`bgHeaderCreate`となります。

## 2. 命名規則・原則
提供するすべての関数、型は`bg`を最初につけます。
typedefによる型定義では語尾に`_t`を付けます。
特に構造体のtypedefでは、型名を`bg<T>_t`となります。
また、構造体の関連関数は`bg<T>Func`のようになります。

構造体は`bg<T>Create`関数と`bg<T>Destroy`関数をもっています。
利用するときは、必ず`bg<T>Create`を呼び出し、使い終わったら`bg<T>Destroy`を呼び出してください。
本番環境では、`bg<T>Create`の戻り値`bgError_t`をチェックしてください。
また、`bgError_t`が`BG_ERROR_OK`でなくても、`bg<T>Destroy`をしてください。

## 3. bg読み取り
`bg`のヘッダーを読み取るサンプルコードを以下に示します。

```c
FILE *in = NULL;
void read_cb(bgOffset_t pos, void *buffer, bgSize_t size) {
  fseek(in, pos, SEEK_SET);
  fread(buffer, size, 1, in);
}

int main(void) {
  // リーダー作成
  in = fopen("bg/minato.bg", "rb");
  bgReader_t reader;
  bgReaderCreate(&reader, read_cb);

  // ヘッダ作成
  bgHeader_t header;
  bgHeaderCreate(&header);
  bgHeaderFetch(&header, &reader); // ヘッダー読み取り

  // headerの利用
  // ...

  bgHeaderDestroy(&header);

  bgReaderDestroy(&reader);
  fclose(in);
  return 0;
}
```

`bgReader_t`を作成してbgを読み取ります。
サンプルでは`stdio.h`のファイルを利用していますが、読み取りコールバック関数を変更して、他のシステムを利用できます。
`bg読み取り構造体`に対して、ファイルからデータを持ってくるには`bg<T>Fetch`関数を用います。
提供する`bg読み取り構造体`の一覧を以下に示します。

|`bg読み取り構造体`|説明|
|-|-|
|bgReader|`bg`をバイナリで読み取る|
|bgHeader|ヘッダー|
|bgNodeContext|ノードコンテキスト|
|bgWayContext|ウェイコンテキスト|
|bgNode|ノード|
|bgWay|ウェイ|

> [!IMPORTANT]
> `bg`データは膨大であるので一部ずつ読み取る利用を想定しています。
> 特にマイコンでは、同じ`bg<T>`を`bg<T>Destroy`せずに、`bg<T>Fetch`を繰り返すようなプログラムにしてください。

## 4. 経路探索コスト計算方法
コスト($ cost $)は、コンテキストの重み($ \bm{weight} $)と、指標値($ \bm{indicator} $)を用いて計算されます。

### 制約
$$
-1 \le \bm{weight}_i \le 1 \\
0 \le \bm{indicator}_i \le 1 \\
0 \le cost \\
$$

### Node
$$
cost = \frac{1}{dim} \sum_{i=1}^{dim}
\left\{ \begin{matrix}
\bm{indicator}_i \times \bm{weight}_i & (\bm{weight}_i \ge 0) \\
(1 - \bm{indicator}_i) \times (-\bm{weight}_i) & (\bm{weight}_i < 0) \\
\end{matrix} \right\}
$$

指標値と重みを掛け合わせる形になります。もし、重みが負なら、指標を反転します。
最後に指標、重みの次元で割ります。

### Way
$$
cost = \frac{length}{dim + 1} \left [ \sum_{i=1}^{dim}
\left\{ \begin{matrix}
\bm{indicator}_i \times \bm{weight}_i & (\bm{weight}_i \ge 0) \\
(1 - \bm{indicator}_i) \times (-\bm{weight}_i) & (\bm{weight}_i < 0) \\
\end{matrix} \right\}
+ 1 \right ]
$$

基本的にはNodeコスト計算は同じです。
最後にWayの長さでコストをスケールします。
また、すべての重みが0でも長さデータが失われないように、必ず1となる仮想の指標値を追加して計算します。

## 5. A\*法 経路探索 準備
### 到達リスト (ハッシュマップ)
到達リスト兼ノード遷移記録には`bgAStarVisited`を利用し、利用者はこれを事前に作成します。
`bgAStarVisited`は、`テーブルバケット`と`チェーンバケット`から構成され、それぞれのバケット数を`table_size`、`chain_size`とします。
`table_size`と`chain_size`の和が容量`capacity`となり、`bg`構造のノード数となれば、確実にすべての範囲で探索が可能になります。
しかし、メモリが足りなければこれを妥協し、`table_size`と`chain_size`を調整する必要があります。

> [!TIP]
> 現状のバケット一つのバイト数は9byteです。

#### (1) `table_size`<br>
ハッシュ法のテーブル部分のサイズです。
この値を大きくすると衝突率が下がって高速に動作しますが、衝突していない未使用バケットが増え、メモリに無駄が発生します。

#### (2) `chain_size`<br>
ハッシュ法のチェイン部分のバケット数です。
ハッシュ値が衝突する場合は必ず必要となります。

> [!IMPORTANT]
> もし、メモリが`bg`構造のノード数まで足りるなら、`table_size`をノード数、`chain_size`を0とできます。
> ハッシュ値はノードのidxが使われ、必ず固有値となります。したがって、衝突することはありません。

作成は以下のようになります。
```c
bgAStarVisitedCreate(&visited, /* table_size */, /* chain_size */);
```

### 優先度付きキュー (最小ヒープ)
優先度付きキューには`bgAStarHeap`を利用し、利用者はこれを事前に作成します。
`bgAStarHeap`では、容量`capacity`を設定します。
`capacity`は`bg`構造のノード数となれば、確実にすべての範囲で探索が可能になります。

ヒープに入るデータは、経路探索での探索済み領域と未探索領域の境界部分のノードに関する情報です。
そのため、到達リストほど多くのメモリは必要ないことがほとんどです。

> [!TIP]
> 現状のヒープノードのバイト数は13byteです。

作成は以下のようになります。
```c
bgAStarHeapCreate(&heap, /* capacity */);
```

### パス (配列)
結果のパスは`bgPath`を利用し、利用者はこれを事前に作成します。
`bgPath`では、容量`capacity`を設定します。
`capacity`は`bg`構造のノード数となれば、確実にすべての範囲で探索が可能になります。

最終的な経路が`bg`構造のノード数の長さになることはほとんどないため、到達リストほど多くのメモリは必要ないことがほとんどです。

> [!TIP]
> 現状のパスの一つ一つのバイト数は4byteです。

作成は以下のようになります。
```c
bgPathCreate(&path, /* capacity */);
```

## 6. A\*法 経路探索
経路探索部は重要な部分であるので実装を説明します。実装全体を以下に示します。
```c
static bgCost_t heuristic(const bgNode_t *node, double goal_lon, double goal_lat) {
  bgCost_t manhattan_dist = fabs(goal_lon - node->lon) + fabs(goal_lat - node->lat);
  return manhattan_dist;
}

bgError_t bgAStar(
  bgReader_t *reader, const bgHeader_t *header,
  const bgNodeContext_t *nc, const bgWayContext_t *wc,
  bgNode_t *node, bgWay_t *way,
  bgAStarVisited_t *visited, bgAStarHeap_t *heap,
  bgIdx_t start_node_idx, bgIdx_t goal_node_idx,
  bgCost_t heuristic_weight,
  bgPath_t *path
) {
  bgError_t err;
  bgCost_t g, h;
  bgIdx_t node_idx;
  bgIdx_t way_begin, way_end;
  bgIdx_t way_idx;
  uint8_t node_transition;
  double goal_lon, goal_lat;

  // ゴール情報を取り出す(heuristicで使うから)
  bgNodeFetch(node, reader, header, goal_node_idx); // nodeはゴールノード
  goal_lon = node->lon;
  goal_lat = node->lat;

  // スタートを最初の後継として追加
  err = bgAStarHeapPush(heap, 0, 0, start_node_idx, 0);
  if(err != BG_ERROR_OK) return err;

  /* A* 探索ループ */
  for(;;) {
    if(heap->size == 0) return BG_ERROR_LOGIC;
    // ヒープ重みが最小の後継を取り出す
    bgAStarHeapTop(heap, NULL, &g, &node_idx, &node_transition);
    bgAStarHeapPop(heap);

    // すでに到達したノードならスキップ、そうでなければ到達を記録
    if(bgAStarVisitedFind(visited, node_idx, NULL)) continue;
    err = bgAStarVisitedInsert(visited, node_idx, node_transition);
    if(err != BG_ERROR_OK) return err;

    // ゴール到達!
    if(node_idx == goal_node_idx) {
      break;
    }

    // 現在ノード情報を取り出す(持ってるウェイを調べるため)
    bgNodeFetch(node, reader, header, node_idx); // nodeは現在ノード
    way_begin = node->way_begin;
    way_end = node->way_end;

    // 現在ノードが持つすべてのウェイに対して
    for(way_idx = way_begin; way_idx < way_end; way_idx++) {
      bgWayFetch(way, reader, header, way_idx); // wayは現在ノードが持つウェイ
      if(bgAStarVisitedFind(visited, way->to, NULL)) continue;
      bgNodeFetch(node, reader, header, way->to); // nodeは後継ノード

      // コスト計算
      g = g + bgNodeCost(node, nc) + bgWayCost(way, wc);
      h = heuristic(node, goal_lon, goal_lat) * heuristic_weight;

      // 後継を追加
      node_transition = way->reverse_idx - node->way_begin;
      err = bgAStarHeapPush(heap, g + h, g, node->idx, node_transition);
      if(err != BG_ERROR_OK) return err;
    }
  }

  /* path作成 */
  for(node_idx = goal_node_idx; node_idx != start_node_idx; node_idx = way->to) { // ゴールからスタートまで、node_transitionをたどる
    err = bgPathPush(path, node_idx); // pathに追加
    if(err != BG_ERROR_OK) return err;

    bgAStarVisitedFind(visited, node_idx, &node_transition);

    bgNodeFetch(node, reader, header, node_idx); // nodeは現在pathに追加したのの
    way_idx = node->way_begin + node_transition;
    bgWayFetch(way, reader, header, way_idx); // wayはnode_transitionが示すもの
  }
  err = bgPathPush(path, start_node_idx);
  if(err != BG_ERROR_OK) return err;
  
  // ゴールからスタートの順なので、反転する
  bgPathReverse(path);

  return BG_ERROR_OK;
}
```
上から順番に見ていきましょう。

### (1) 引数
```c
bgError_t bgAStar(
  bgReader_t *reader, const bgHeader_t *header,
  const bgNodeContext_t *nc, const bgWayContext_t *wc,
  bgNode_t *node, bgWay_t *way,
  bgAStarVisited_t *visited, bgAStarHeap_t *heap,
  bgIdx_t start_node_idx, bgIdx_t goal_node_idx,
  bgCost_t heuristic_weight,
  bgPath_t *path
)
```
`bg`を読み取るためのデータ、到達リスト、優先度付きキュー、パスを受け取ります。

`heuristic_weight`はA*法のヒューリスティックコストの重みです。
この値が大きければ、貪欲な探索となり高速で低消費メモリになります。
しかし、指標を用いたコスト計算の恩恵が小さくなってしまうでしょう。

> [!TIP]
> あるノードのヒューリスティックコストが、そのノードからゴールまでの総コスト以下であるとき、
> A\*法で最小コスト経路となる保証されます。
>
> 現状はヒューリスティックコストはマンハッタン距離を用いていることや、
> 面倒なコスト計算があることから、
> 最小コスト経路を保証するのは難しいです。

> [!NOTE]
> `heuristic_weight`が0であるとき、`bgAStar`はダイクストラ法と同じ振る舞いをします。

### (2) ヒューリスティック
```c
static bgCost_t heuristic(const bgNode_t *node, double goal_lon, double goal_lat) {
  bgCost_t manhattan_dist = fabs(goal_lon - node->lon) + fabs(goal_lat - node->lat);
  return manhattan_dist;
}
```

```c
// ゴール情報を取り出す(heuristicで使うから)
bgNodeFetch(node, reader, header, goal_node_idx); // nodeはゴールノード
goal_lon = node->lon;
goal_lat = node->lat;
```

> [!CAUTION]
> grapher、beltgraph両方で距離には緯度経度の角度の距離を用いています。
> これは正しい距離計算ではないです。
> より広い地図を用いる場合や、緯度、経度の符号が変化する場合に問題になります。

A\*法のヒューリスティックコストは、ノードからゴールノードまでのマンハッタン距離です。
ゴールノードの位置を知る必要があるので`bgAStar`最初にデータをフェッチします。

> このタイミングで、`bgNodeFetch`により変数`node`の中身がゴールノードになることに注意してください。

### (3) スタートノードをヒープに追加
```c
// スタートを最初の後継として追加
err = bgAStarHeapPush(heap, 0, 0, start_node_idx, 0);
if(err != BG_ERROR_OK) return err;
```
スタートをヒープに追加します。
コスト系の値は0にしておきます。
コストは`(5) ヒープ取り出し`、`(10) コスト計算`で説明します。
スタートの`node_transition`は使うことは一切ありませんが、0としておきます。
`node_transition`については`(11) 後継をヒープに追加`で説明します。

### (4) 探索ループ
```c
/* A\* 探索ループ */
for(;;) {
  // ... 
}
```
無限ループです。ループのうちの一回の処理は、一つのノードに対する処理です。

### (5) ヒープ取り出し
```c
if(heap->size == 0) return BG_ERROR_LOGIC;
// ヒープ重みが最小の後継を取り出す
bgAStarHeapTop(heap, NULL, &g, &node_idx, &node_transition);
bgAStarHeapPop(heap);
```
ヒープが空であったら、ゴールに到達せずに、到達可能なすべてのノードを探索したことになります。
これは、スタートとゴールがそれぞれ離れ小島になっていたら起こります。

そうでなければ、ヒープ重みが最小であるノード情報を取り出します。
A\*法でノード$ n $のヒープ重み$ f(n) $は以下の式です。
$$
f(n) = g(n) + h(n)
$$
$ g(n) $はスタートからノード$ n $までの総コストです。<br>
$ h(n) $はノード$ n $からゴールまでのヒューリスティックコストです。
ゴールに近いほど小さくなります。<br>

$ g(n) $に注目して、`(4) 探索ループ`と組み合わせて考えると、`スタートからの総コスト`が小さい順に到達可能なノード情報を取り出すようになっています。`スタートからの総コスト`が小さい順にノードを取り出すということは、それぞれのノードに対して順番に、最小コストで到達している、ということになります。これを繰り返してゴールに到達したら、それは最小コストの経路ということですね。<br>
$ h(n) $に注目すると、ゴールに近い方ほど、$ f(n) $が小さくなります。つまり、ゴールに向かうノードの方が、ヒープの最小を選ぶという動作で選ばれやすくなります。これがA*法が効率が良い理由です。

実際にこの計算をする部分は`(10) コスト計算`で説明します。

### (6) 到達記録
```c
// すでに到達したノードならスキップ、そうでなければ到達を記録
if(bgAStarVisitedFind(visited, node_idx, NULL)) continue;
err = bgAStarVisitedInsert(visited, node_idx, node_transition);
if(err != BG_ERROR_OK) return err;
```
すでに到達していたら、何もせずに次の探索ループへ行きます。

そうでなければ、到達を記録します。
このとき、あとで経路を復元するために、ノードの移動`node_transition`を記録します。
`node_transition`については`(11) 後継をヒープに追加`で説明します。

### (7) ゴール判定
```c
// ゴール到達!
if(node_idx == goal_node_idx) {
  break;
}
```
ゴールに到達したときのみ、この探索ループを正常に抜けることができます。

### (8) 現在ノード取り出し
```c
// 現在ノード情報を取り出す(持ってるウェイを調べるため)
bgNodeFetch(node, reader, header, node_idx); // nodeは現在ノード
way_begin = node->way_begin;
way_end = node->way_end;
```
`(5) ヒープ取り出し`で取り出したノードを`現在ノード`としています。
`現在ノード`から、次のノードに進むために、持っているウェイの情報をとりだします。

> このタイミングで、`bgNodeFetch`により変数`node`の中身が`現在ノード`になることに注意してください。

### (9) 現在ノードのウェイ取り出し
```c
// 現在ノードが持つすべてのウェイに対して
for(way_idx = way_begin; way_idx < way_end; way_idx++) {
  bgWayFetch(way, reader, header, way_idx); // wayは現在ノードが持つウェイ
  if(bgAStarVisitedFind(visited, way->to, NULL)) continue;
  bgNodeFetch(node, reader, header, way->to); // nodeは後継ノード

  // ...
}
```
`現在ノード`が持つ、すべてのウェイについて反復処理します。
そこでは、ウェイを取り出し、そのウェイが示す接続先ノードを取り出します。
このとき、接続先ノードを`後継ノード`としています。

`後継ノード`が到達済みであったらスキップします。この処理は`(6) 到達記録`でも出てきました。
`(6) 到達記録`のスキップのみでもアルゴリズムとして正しく動きますが、ヒープの負荷を下げるためにここでもスキップします。

ここのスキップは到達済みのノードを絶対に再探索しないためのものではありません。
というのも、ヒープに入ってはいるが、まだ到達していない同じ`後継ノード`がいる場合、スキップできないからです。
その場合、同じ`後継ノード`について、すでにヒープにいるコストと、これから追加するコストとで比較が発生します。
ヒープによって小さいほうが選ばれます。

> このタイミングで、`bgNodeFetch`により変数`node`の中身が`後継ノード`になることに注意してください。

### (10) コスト計算
```c
// コスト計算
g = g + bgNodeCost(node, nc) + bgWayCost(way, wc);
h = heuristic(node, goal_lon, goal_lat) * heuristic_weight;
```
`(5) ヒープ取り出し`で出てきた$ g(n) $、$ h(n) $の計算です。

`後継ノード`の$ g(n) $は、`現在ノード`の$ g(m) $と、ウェイのコスト、`後継ノード`のコストの和です。
ここの、ウェイのコスト、`後継ノード`のコストは`4. 経路探索コスト計算方法`で説明したものです。

$ h(n) $は`(2) ヒューリスティック`のものです。
`heuristic_weight`は`(1) 引数`で説明したものです。

### (11) 後継ノードをヒープに追加
```c
// 後継を追加
node_transition = way->reverse_idx - node->way_begin;
err = bgAStarHeapPush(heap, g + h, g, node->idx, node_transition);
if(err != BG_ERROR_OK) return err;
```
ヒープの重み$ f(n) $は`(5) ヒープ取り出し`の通りです。

`(6) 到達記録`で説明したように、経路復元に`node_transition`を計算します。
単純な実装のノードの遷移の記録では、単純に`現在ノード`を前回のノードとして`後継ノード`に結びつけます。
しかし、現在ノードを指し示す`bgIdx_t`は32bit整数で4byteあります。
ここで、`後継ノード`の持っているウェイの内、`現在ノード`に向かうウェイを記録します。
一般的な道路でノードの出次数が255を超えることはめったにありません。
したがって、8bit整数で1byteデータで保存できます。
grapherはこのためだけに、`現在ノード`から`後継ノード`のウェイに、
逆向きのウェイである`後継ノード`から`現在ノード`のウェイ情報を事前計算しています。

> [!CAUTION]
> 出次数が255を超えるノードが存在する地図データでは予期せぬ動作をするでしょう。

すべての`後継ノード`をヒープに追加できたら、探索ループの一つがようやく終了です。
`(4) 探索ループ`に戻ります。

### (12) 経路作成
```c
/* path作成 */
for(node_idx = goal_node_idx; node_idx != start_node_idx; node_idx = way->to) { // ゴールからスタートまで、node_transitionをたどる
  err = bgPathPush(path, node_idx); // pathに追加
  if(err != BG_ERROR_OK) return err;

  bgAStarVisitedFind(visited, node_idx, &node_transition);

  bgNodeFetch(node, reader, header, node_idx); // nodeは現在pathに追加したのの
  way_idx = node->way_begin + node_transition;
  bgWayFetch(way, reader, header, way_idx); // wayはnode_transitionが示すもの
}
err = bgPathPush(path, start_node_idx);
if(err != BG_ERROR_OK) return err;
  
// ゴールからスタートの順なので、反転する
bgPathReverse(path);
```
ここに制御が到達できるのは`(7) ゴール判定`で探索ループを抜けた時だけです。

`(6) 到達記録`、`(11) 後継ノードをヒープに追加`で説明したように、
それぞれのノードは前回のノード情報を記録しています。
つまり、ゴールからスタートに向かう方向で経路を復元できます。

経路長はこの時点ではわからいので、`path`をスタックとして扱い、
ゴールから通ったノードを順番に積みます。

最後に、ゴールから順番にノードを積んだ`path`をリバースして、
スタートからゴールの経路にします。
ついに経路作成に成功しまた!!ふぅ。
