# beltgraphの優先度付きキューの実装

## 1. データ構造
```c
struct bgAStarHeap {
  // SoAの構造です。

  // 指標値は厳密な数値ではないので、
  // weightとcostは半精度浮動小数点を使えるかもしれません
  bgCost_t *weight;
  bgCost_t *cost;
  bgIdx_t *node_idx;
  uint8_t *node_transition;

  bgIdx_t capacity;
  bgIdx_t size;
  bgIdx_t peak_size;
};
```
優先度付きキューは最小バイナリヒープです。

ヒープの要素は`weight`、`cost`、`node_idx`、`node_transition`を持っていて、それぞれ別の配列となっています。
このSoA構造をとる理由は、AoSの場合、構造体対してコンパイラがアライメント・パディングをした場合、メモリに無駄ができるからです。(高速ではあるが)

`weight`はヒープの要素の重み、`cost`は経路でのこのノードまでの実コスト和、
`node_idx`は探索予定のノード、`node_transition`は経路復元用のデータです。

ヒューリスティックを足した値`weight`と、コストの和を保存する`cost`は、
一緒にすることはできません。

## 2. プッシュ
```c
bgError_t bgAStarHeapPush(
  bgAStarHeap_t *heap,
  bgCost_t weight,
  bgCost_t cost, bgIdx_t node_idx, uint8_t node_transition
) {
  if(heap->size == heap->capacity) {
    return BG_ERROR_CAP;
  }

  const bgIdx_t back = heap->size++;
  bgIdx_t p;
  bgIdx_t c = back;

  while(c != 0) {
    p = (c - 1) >> 1;
    if(heap->weight[p] <= weight) break;

    heap->weight[c] = heap->weight[p];
    heap->cost[c] = heap->cost[p];
    heap->node_idx[c] = heap->node_idx[p];
    heap->node_transition[c] = heap->node_transition[p];
    
    c = p;
  }

  heap->weight[c] = weight;
  heap->cost[c] = cost;
  heap->node_idx[c] = node_idx;
  heap->node_transition[c] = node_transition;

  if(heap->peak_size < heap->size) {
    heap->peak_size = heap->size;
  }
  return BG_ERROR_OK;
}
```
末尾に要素を追加して、最小ヒープとなるように、追加した要素を上にシフトするイメージです。

(1) 末尾に追加したものが最初の子<br>
(2) 親の重みより子の重みが大きい場合(5)へ行く<br>
(3) 親を子の場所にコピーし、今の親を子とする<br>
(4) (2)へ行く<br>
(5) 子の場所に追加データを書く<br>

この方法なら、スワップで実装するより、コピーの回数が少ないです。

## 3. ポップ
```c
bgError_t bgAStarHeapPop(bgAStarHeap_t *heap) {
  if(heap->size == 0) {
    return BG_ERROR_LOGIC;
  }

  const bgIdx_t back = --heap->size;
  bgIdx_t p = 0;
  bgIdx_t c;

  while((c = (p << 1) + 1) < heap->size) {
    if(c + 1 < heap->size && heap->weight[c] > heap->weight[c + 1]) {
      c = c + 1;
    }
    if(heap->weight[back] <= heap->weight[c]) break;

    heap->weight[p] = heap->weight[c];
    heap->cost[p] = heap->cost[c];
    heap->node_idx[p] = heap->node_idx[c];
    heap->node_transition[p] = heap->node_transition[c];

    p = c;
  }

  heap->weight[p] = heap->weight[back];
  heap->cost[p] = heap->cost[back];
  heap->node_idx[p] = heap->node_idx[back];
  heap->node_transition[p] = heap->node_transition[back];

  return BG_ERROR_OK;
}
```
先頭の要素を削除して、そこに末尾の要素を持ってきて、それを下にシフトするイメージです。

(1) 先頭に持ってきた要素が最初の親<br>
(2) 親の重みより子の重みが大きい場合(5)へ行く<br>
(3) 子を親の場所にコピーし、今の子を親とする<br>
(4) (2)へ行く<br>
(5) 親の場所に先頭に持ってきた要素データを書く<br>

この方法なら、スワップで実装するより、コピーの回数が少ないです。
