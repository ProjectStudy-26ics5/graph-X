# beltgraph
grapherで作成したbgを読み、経路探索処理を提供するCライブラリです。

## 1. 表記方法
このREADMEでは、構造体名を`<T>`として、`bg<T>Create`のような表記をします。
例えば、構造体名が`Header`なら、`bgHeaderCreate`となります。

## 2. 利用方法
`beltgraph.h`をインクルードし、CMakeから`PS::beltgraph`をリンクすると、すべての機能が使えるようになります。

提供するすべての関数、型は`bg`を最初につけます。
typedefによる型定義では語尾に`_t`を付けます。
特に構造体のtypedefでは、型名を`bg<T>_t`となります。
また、構造体の関連関数は`bg<T>Func`のようになります。

提供する構造体は`bg<T>Create`関数と`bg<T>Destroy`関数をもっています。
利用するときは、必ず`bg<T>Create`を呼び出し、使い終わったら`bg<T>Destroy`を呼び出してください。
本番環境では、`bg<T>Create`の戻り値`bgError_t`をチェックしてください。
`bg<T>Create`で失敗した場合、自動的に`bg<T>Destroy`が呼ばれます。

bgを読み取って経路探索をする、サンプルコードを以下に示します。部分的な説明を続く章で説明します。
```c
#include <beltgraph.h>
#include <stdio.h>

FILE *in = NULL;
void read_cb(bgOffset_t pos, void *buffer, bgSize_t size) {
  fseek(in, pos, SEEK_SET);
  fread(buffer, size, 1, in);
}

int main(void) {
  bgError_t err;
  
  /* 初期化 */
  in = fopen("bg/minato.bg", "rb");
  bgReader_t reader;
  bgReaderCreate(&reader, read_cb);

  bgHeader_t header;
  bgHeaderCreate(&header);
  bgHeaderFetch(&header, &reader);
  bgNodeContext_t nc;
  bgNodeContextCreate(&nc, &header);
  bgNodeContextFetch(&nc, &reader, &header);
  bgWayContext_t wc;
  bgWayContextCreate(&wc, &header);
  bgWayContextFetch(&wc, &reader, &header);

  bgNode_t node;
  bgNodeCreate(&node, &header);
  bgWay_t way;
  bgWayCreate(&way, &header);

  bgAStarVisited_t visited;
  bgAStarVisitedCreate(&visited, header.node_count, 0);
  bgAStarHeap_t heap;
  bgAStarHeapCreate(&heap, header.node_count);
  bgPath_t path;
  bgPathCreate(&path, header.node_count);

  /* 経路探索の重み設定 */
  for(bgIdx_t i = 0; i < wc.dim; i++) {
    bgWayContextSetWeight(&wc, i, 0);
  }
  for(bgIdx_t i = 0; i < nc.dim; i++) {
    bgNodeContextSetWeight(&nc, i, 0);
  }

  /* スタートとゴール設定 */
  bgNodeFetchNearest(&node, &reader, &header, 139.751, 35.635);
  bgIdx_t start_node_idx = node.idx;
  bgNodeFetchNearest(&node, &reader, &header, 139.800, 35.635);
  bgIdx_t goal_node_idx = node.idx;

  /* 経路探索 */
  err = bgAStar(
    &reader, &header,
    &nc, &wc,
    &node, &way,
    &visited, &heap,
    start_node_idx, goal_node_idx,
    1,
    &path
  );

  /* 経路探索に成功したら経路を表示 */
  if(err == BG_ERROR_OK) {
    printf("path = {");
    for(bgIdx_t i = 0; i < path.size; i++) {
      printf("%u -> ", path.node_idx_order[i]);
    }
    printf("\b\b\b\b}    \n\n");
  }else {
    printf("Error !\n");
  }

  /* リソースの開放 */
  fclose(in);
  bgReaderDestroy(&reader);

  bgHeaderDestroy(&header);
  bgNodeContextDestroy(&nc);
  bgWayContextDestroy(&wc);

  bgNodeDestroy(&node);
  bgWayDestroy(&way);

  bgAStarVisitedDestroy(&visited);
  bgAStarHeapDestroy(&heap);
  bgPathDestroy(&path);
  return 0;
}
```

## 3. bg読み取り
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

  fclose(in);
  bgReaderDestroy(&reader);

  bgHeaderDestroy(&header);

  // ...
}
```

`bgReader_t`を作成してbgを読み取ります。
サンプルでは`stdio.h`のファイルを利用していますが、読み取りコールバック関数を変更して、他のシステムを利用できます。

`bg読み取り構造体`に対して、ファイルからデータを持ってくるには`bg<T>Fetch`関数を用います。
提供する`bg読み取り構造体`の一覧を以下に示します。
bgのデータ構造の詳細は`appendix/bg_binary.md`を参照してください。

* bgHeader
* bgNodeContext
* bgWayContext
* bgNode
* bgWay

> [!IMPORTANT]
> `bg`データは膨大であるので一部ずつ読み取る利用を想定しています。
> 特にマイコンでは、同じ`bg<T>`を`bg<T>Destroy`せずに、`bg<T>Fetch`を繰り返すようなプログラムにしてください。

## 4. 到達リスト (ハッシュマップ)
```c
bgAStarVisited_t visited;
bgAStarVisitedCreate(&visited, /* table_size */, /* chain_size */);

// ...

bgAStarVisitedDestroy(&visited);
```
到達リスト兼ノード遷移記録には`bgAStarVisited`を利用し、利用者はこれを事前に作成します。
`bgAStarVisited`は、テーブルバケットとチェーンバケットから構成され、それぞれのバケット数を`table_size`、`chain_size`とします。
`table_size`と`chain_size`の和が容量`capacity`となり、`bg`構造のノード数となれば、確実にすべての範囲で探索が可能になります。
しかし、メモリが足りなければこれを妥協し、`table_size`と`chain_size`を調整する必要があります。

> [!TIP]
> 現状のバケット一つのバイト数は9byteです。

### (1) `table_size`<br>
ハッシュ法のテーブル部分のサイズです。
この値を大きくすると衝突率が下がって高速に動作しますが、衝突していない未使用バケットが増え、メモリに無駄が発生します。

### (2) `chain_size`<br>
ハッシュ法のチェイン部分のバケット数です。
ハッシュ値が衝突する場合は必ず必要となります。

> [!IMPORTANT]
> もし、メモリが`bg`構造のノード数まで足りるなら、`table_size`をノード数、`chain_size`を0とできます。
> ハッシュ値はノードのidxが使われ、必ず固有値となります。したがって、衝突することはありません。

## 5. 優先度付きキュー (バイナリヒープ)
```c
bgAStarHeap_t heap;
bgAStarHeapCreate(&heap, /* capacity */);

// ...

bgAStarHeapDestroy(&heap);
```
優先度付きキューには`bgAStarHeap`を利用し、利用者はこれを事前に作成します。
`bgAStarHeap`では、容量`capacity`を設定します。
`capacity`は`bg`構造のノード数となれば、確実にすべての範囲で探索が可能になります。

ヒープに入るデータは、経路探索での探索済み領域と未探索領域の境界部分のノードに関する情報です。
そのため、到達リストほど多くのメモリは必要ないことがほとんどです。

> [!TIP]
> 現状のヒープノードのバイト数は13byteです。

## 6. パス (配列)
```c
bgPath_t path;
bgPathCreate(&path, /* capacity */);

// ...

/* 経路探索に成功したら経路を表示 */
if(err == BG_ERROR_OK) {
  printf("path = {");
  for(bgIdx_t i = 0; i < path.size; i++) {
    printf("%u -> ", path.node_idx_order[i]);
  }
  printf("\b\b\b\b}    \n\n");
}else {
  printf("Error !\n");
}

// ...

bgPathDestroy(&path);
```
結果のパスは`bgPath`を利用し、利用者はこれを事前に作成します。
`bgPath`では、容量`capacity`を設定します。
`capacity`は`bg`構造のノード数となれば、確実にすべての範囲で探索が可能になります。

最終的な経路が`bg`構造のノード数の長さになることはほとんどないため、到達リストほど多くのメモリは必要ないことがほとんどです。

> [!TIP]
> 現状のパスの一つ一つのバイト数は4byteです。

`bgPath`はスタートからゴールまでのノードのidx配列を`bgPath::node_idx_order`に持っていて、経路長は`bgPath::size`です。

## 7. 重み
```c
/* 経路探索の重み設定 */
for(bgIdx_t i = 0; i < wc.dim; i++) {
  bgWayContextSetWeight(&wc, i, 0);
}
for(bgIdx_t i = 0; i < nc.dim; i++) {
  bgNodeContextSetWeight(&nc, i, 0);
}
```
bgの経路探索では指標と重みを用いて特殊なコスト計算をします。
詳細は`appendix/bg_cost.md`を参照してください。

ノードコンテキスト、ウェイコンテキストそれぞれの指標値に対応する重みを設定しています。

## 8. 探索探索
```c
/* 経路探索 */
err = bgAStar(
  &reader, &header,
  &nc, &wc,
  &node, &way,
  &visited, &heap,
  start_node_idx, goal_node_idx,
  1,
  &path
);
```
bg読み取りに必要なデータを渡します。
`node`、`way`は探索時の読み取りで使われるバッファです。
続いて、準備した、到達リスト、ヒープ、パスを渡します。
スタート地点とゴール地点のidxを渡します。

`err`は経路探索に成功したら`BG_ERROR_OK`となります。

詳細は`appendix/bg_aster.md`を参照してください。
