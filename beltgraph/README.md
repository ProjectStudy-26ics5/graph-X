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
`bg<T>Create`で失敗した場合、自動的に`bg<T>Destroy`が呼ばれます。

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
