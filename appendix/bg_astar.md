# beltgraphのbgAStarの実装
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

## (1) 引数
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

`heuristic_weight`はA\*法のヒューリスティックコストの重みです。
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

## (2) ヒューリスティックに用いるデータ
```c
// ゴール情報を取り出す(heuristicで使うから)
bgNodeFetch(node, reader, header, goal_node_idx); // nodeはゴールノード
goal_lon = node->lon;
goal_lat = node->lat;
```

A\*法のヒューリスティックコストは多くの場合、ゴールまでの距離です。
ゴールノードの位置を知る必要があるので`bgAStar`最初にデータをフェッチします。

> このタイミングで、`bgNodeFetch`により変数`node`の中身がゴールノードになることに注意してください。

## (3) スタートノードをヒープに追加
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

## (4) 探索ループ
```c
/* A\* 探索ループ */
for(;;) {
  // ... 
}
```
無限ループです。ループのうちの一回の処理は、一つのノードに対する処理です。

## (5) ヒープ取り出し
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
$ h(n) $に注目すると、ゴールに近い方ほど、$ f(n) $が小さくなります。つまり、ゴールに向かうノードの方が、ヒープの最小を選ぶという動作で選ばれやすくなります。これがA\*法が効率が良い理由です。

実際にこの計算をする部分は`(10) コスト計算`で説明します。

## (6) 到達記録
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

## (7) ゴール判定
```c
// ゴール到達!
if(node_idx == goal_node_idx) {
  break;
}
```
ゴールに到達したときのみ、この探索ループを正常に抜けることができます。

## (8) 現在ノード取り出し
```c
// 現在ノード情報を取り出す(持ってるウェイを調べるため)
bgNodeFetch(node, reader, header, node_idx); // nodeは現在ノード
way_begin = node->way_begin;
way_end = node->way_end;
```
`(5) ヒープ取り出し`で取り出したノードを`現在ノード`としています。
`現在ノード`から、次のノードに進むために、持っているウェイの情報をとりだします。

> このタイミングで、`bgNodeFetch`により変数`node`の中身が`現在ノード`になることに注意してください。

## (9) 現在ノードのウェイ取り出し
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

## (10) コスト計算
```c
// コスト計算
g = g + bgNodeCost(node, nc) + bgWayCost(way, wc);
h = heuristic(node, goal_lon, goal_lat) * heuristic_weight;
```
`(5) ヒープ取り出し`で出てきた$ g(n) $、$ h(n) $の計算です。

`後継ノード`の$ g(n) $は、`現在ノード`の$ g(m) $と、ウェイのコスト、`後継ノード`のコストの和です。
ここの、ウェイのコスト、`後継ノード`のコストは`appendix/bg_cost.md`で説明しています。

`後継ノード`の$ h(n) $はヒューリスティックコストで、`appendix/bg_cost.md`で説明しています。
`heuristic_weight`は`(1) 引数`で説明したものです。

## (11) 後継ノードをヒープに追加
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

## (12) 経路作成
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
