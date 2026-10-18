# beltgraph
grapherで作成した`bg`を読み、経路探索処理を提供するCライブラリです。

## 1. bg読み取り
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
構造体`bg<T>`に対して、ファイルからデータを持ってくるには`bg<T>Fetch`関数を用います。
提供するbg読み取り用の構造体の一覧を以下に示します。

|構造体|説明|
|-|-|
|bgReader|`bg`をバイナリで読み取る|
|bgHeader|ヘッダー|
|bgNodeContext|ノードコンテキスト|
|bgWayContext|ウェイコンテキスト|
|bgNode|ノード|
|bgWay|ウェイ|

> [!IMPORTANT]
> ここの構造体`bg<T>`では、対応する`bg<T>Create`関数と`bg<T>Destroy`関数をもっています。
> 利用するときは、必ず`bg<T>Create`を呼び出し、使い終わったら`bg<T>Destroy`を呼び出してください。
> 本番環境では、`bg<T>Create`の戻り値`bgError_t`をチェックしてください。

> [!IMPORTANT]
> `bg`データは膨大であるので一部ずつ読み取る利用を想定しています。
> 特にマイコンでは、同じ`bg<T>`を`bg<T>Destroy`せずに、`bg<T>Fetch`を繰り返すようなプログラムにしてください。

## 2. 経路探索コスト計算方法
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

## 3. 経路探索
### 到達リスト
到達リスト兼ノード遷移記録には`bgVisited`を利用しています。
`bgVisited`の利用は、ライブラリ側がメインですが、構築は利用者が行います。
`bgVisited`は、`table backet`と`chain backet`から構成され、バケット数は`table backet`数と`chain backet`数の和となります。
バケット数は`bg`構造のノード数となれば、確実にすべての範囲で探索が可能になります。
しかし、メモリが足りなければこれを妥協し、`table backet`数と`chain backet`数を調整する必要があります。

> [!TIP]
> 現状のバケット一つのバイト数は9byteです。

(1) `table backet`数<br>
ハッシュ法のテーブル数です。
この値を大きくすると衝突率が下がって高速に動作しますが、衝突していない未使用バケットが増え、メモリに無駄が発生します。

(2) `chain backet`数<br>
ハッシュ法のチェイン部分のバケット数です。
ハッシュ値が衝突する場合は必ず必要となります。

> [!IMPORTANT]
> もし、メモリが`bg`構造のノード数まで足りるなら、`table backet`数をノード数、`chain backet`数を0とできます。
> ハッシュ値はノードのidxが使われ、必ず固有値となります。したがって、衝突することはありません。
