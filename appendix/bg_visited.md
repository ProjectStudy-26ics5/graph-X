# beltgraphの到達リストの実装

## 1. データ構造
```c
struct bgAStarVisited {
  // SoAの構造です。
  
  // hash値はノードのidxです
  bgIdx_t *hash;

  // chainは
  // bgAStarVisited::sizeであるときは、chain終了
  // 0であるときは、このバケットが未割当
  bgIdx_t *chain;

  // Nodeが持っているWayのうち、前回のノードにつながるもの
  // 出次数が255を超えることはめったにないため
  // 8bit(1byte)のデータに圧縮できます
  uint8_t *node_transition;

  bgIdx_t capacity;
  bgIdx_t size;
  bgIdx_t table_size;
  bgIdx_t chain_top;
};
```
到達リストはチェイン法のハッシュマップです。

バケットは`hash`、`chain`、`node_transition`を持っていて、それぞれ別の配列となっています。
このSoA構造をとる理由は、以下の2つです。
* AoSの場合、構造体対してコンパイラがアライメント・パディングをした場合、メモリに無駄ができる。(高速ではあるが)
* バケットの探索時に、`hash`と`chain`が主に使われるため、`node_transition`によるCPUキャッシュメモリの無駄を防げる可能性がある。

`hash`はハッシュ値、`chain`はチェイン先のバケットのインデックス、
`node_transition`は経路復元用のデータです。
バケットに探索時のコストを記録しないのかと思うかもしれませんが、
優先度キューが変わりに一時的に行っているため必要ありません。

バケットは、テーブル領域とチェイン領域に、理論的に分けられます。
インデックスが`table_size`未満であるものはテーブル領域のバケット、
そうでなかったらチェイン領域のバケットとなります。

## 2. バケットの状態
```c
#define BUCKET_END_SIGN(visited) ((visited)->capacity)
#define BUCKET_UNALLOCATED_SIGN 0
#define UNALLOCATED_BUCKET(visited, i) ((visited)->chain[i] == BUCKET_UNALLOCATED_SIGN)
```
バケットは`chain`に、チェイン先のバケットだけでなく、状態を持ちます。
`chain`は符号なし整数で、チェイン先になりえない数値を状態に割り当てています。

`chain`が0である場合、まだ割り当てられていないバケットです。
チェイン先のインデックスは通常は1以上になるため、0は安全です。
というのも、テーブル領域は必ず1以上ないと機能せず、
そうすると、チェイン領域のバケットのインデックスは1以上だからです。

`chain`がバケット配列の容量数`capacity`であるときは、チェインの終わりを示します。
配列の要素番号は0から、要素数-1までなので、当然インデックスに`capacity`は使われませんね。

未割当のバケットに容量数+1ではなく、0を使う理由は、
`capacity`と同じ整数型で同じ表現範囲に収めるためです。

## 3. バケットの探索
```c
int bgAStarVisitedFind(const bgAStarVisited_t *visited, bgIdx_t hash, uint8_t *node_transition) {
  for(bgIdx_t i = hash % visited->table_size; i != BUCKET_END_SIGN(visited); i = visited->chain[i]) {
    if(UNALLOCATED_BUCKET(visited, i)) break;

    if(visited->hash[i] == hash) {
      if(node_transition != NULL) {
        *node_transition = visited->node_transition[i];
      }
      return 1;
    }
  }

  return 0;
}
```
`i`をテーブル領域でのインデックスから、チェインをたどって、チェイン終了バケットになるまで繰り返します。

もし、未割当てのバケットだったら終了します。
これは、テーブル領域のバケットのみ起こりえます。

バケットのハッシュ値が探索するハッシュ値と一致した場合、
見つかったことを返します。

## 4. バケットの追加
```c
bgError_t bgAStarVisitedInsert(bgAStarVisited_t *visited, bgIdx_t hash, uint8_t node_transition) {
  // 要素がすでに存在しても追加します
  bgIdx_t i = hash % visited->table_size;

  if(UNALLOCATED_BUCKET(visited, i)) {
    visited->hash[i] = hash;
    visited->chain[i] = BUCKET_END_SIGN(visited);
    visited->node_transition[i] = node_transition;

    visited->size++;
    return BG_ERROR_OK;
  }

  if(visited->chain_top == visited->capacity) return BG_ERROR_CAP;
  bgIdx_t j = visited->chain_top++;

  visited->hash[j] = hash;
  visited->chain[j] = visited->chain[i];
  visited->node_transition[j] = node_transition;

  visited->chain[i] = j;

  visited->size++;
  return BG_ERROR_OK;
}
```
`i`をテーブル領域のインデックスとします。

このテーブル領域のバケットが未割あてなら、そこに書き込んで終了します。

そうでなければ、チェイン領域にスタックのように追加します。
すでに割り当てられていたテーブル領域のバケットの、チェイン先をこのバケットに追加し、
テーブル領域のバケットはこのバケットをチェイン先に書き換えます。
