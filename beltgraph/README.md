# beltgraph
grapherで作成した`bg`を読み、経路探索処理を提供するCライブラリです。

## 1. メモリ管理
提供するすべての構造体`<T>`には、対応する`<T>Create`関数と`<T>Destroy`関数をもっています。
利用するときは、必ず`<T>Create`を呼び出し、使い終わったら`<T>Destroy`を呼び出してください。

## 2. bg読み取り
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
構造体`<T>`に対して、ファイルからデータを持ってくるには`<T>Fetch`関数を用います。

> [!IMPORTANT]
> `bg`データは膨大であるので一部ずつ読み取る利用を想定しています。
> 特にマイコンでは、同じ`<T>`を`<T>Destroy`せずに、`<T>Fetch`を繰り返すようなプログラムにしてください。

## 3. 構造体一覧
提供する構造体の一覧を以下に示します。
いずれも、このREADMEの構造体`<T>`に当てはまります。

|構造体|説明|
|-|-|
|bgReader|`bg`をバイナリで読み取る|
|bgHeader|ヘッダー|
|bgNodeContext|ノードコンテキスト|
|bgWayContext|ウェイコンテキスト|
|bgNode|ノード|
|bgWay|ウェイ|

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
