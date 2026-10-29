# 指標・コスト・重み
grapherおよびbeltgraphの指標、コスト、重みについて説明します。

## 1. 指標計算
`beltgraph/README.md`で説明した、指標定義での評価値($ \bm{E} $)から、
指標値($ \bm{indicator} $)に計算する過程を示します。

$$
\bm{E}_i :=
\left\{ \begin{matrix}
\bm{E}_i & (\bm{E}_i \ge 0) \\
0 & (\bm{E}_i < 0) \\
\end{matrix} \right\}
$$
$$
\bm{indicator}_i = 
\left\{ \begin{matrix}
\displaystyle \frac{\bm{E}_i - \bm{Emin}_i}{\bm{Emax}_i - \bm{Emin}_i} & (\bm{Emax}_i - \bm{Emin}_i \ne 0) \\
1 & (\bm{Emax}_i - \bm{Emin}_i = 0) \\
\end{matrix} \right\}
$$

評価値、指標値は0以上に強制します。正規化で0除算が発生する場合は指標値を1とします。

> [!NOTE]
> 正規化で0除算が発生する場合は、その指標が機能していない状態です。

## 2. コスト計算
コスト($ cost $)は、コンテキストの重み($ \bm{weight} $)と、指標値($ \bm{indicator} $)を用いて計算されます。
ノードとウェイで、同じ名称を利用していますが別のものです。

### 制約
$$
-1 \le \bm{weight}_i \le 1 \\
0 \le \bm{indicator}_i \le 1 \\
0 \le cost \\
$$

> [!NOTE]
> コストが0以上である理由はダイクストラ法(A\*法の根底)によるものです。
> 負のコストを扱える、ベルマン・フォード法とその派生を利用してもよいですが、
> ダイクストラ法のほうが計算が容易で速く、コスト計算側で解決する方が単純であると考えたためです。

### ノード
```c
bgCost_t bgNodeCost(const bgNode_t *node, const bgNodeContext_t *nc) {
  bgCost_t c = 0;

  for(bgIdx_t i = 0; i < nc->dim; i++) {
    if(nc->weights[i] > 0) {
      c += node->indicators[i] * nc->weights[i];
    }else {
      c += (1 - node->indicators[i]) * -nc->weights[i];
    }
  }

  c /= nc->dim;
  return c;
}
```

$$
cost = \frac{1}{dim} \sum_{i=1}^{dim}
\left\{ \begin{matrix}
\bm{indicator}_i \times \bm{weight}_i & (\bm{weight}_i \ge 0) \\
(1 - \bm{indicator}_i) \times (-\bm{weight}_i) & (\bm{weight}_i < 0) \\
\end{matrix} \right\}
$$

指標値と重みを掛け合わせる形になります。もし、重みが負なら、指標を反転します。
最後に指標、重みの次元で割ります。

### ウェイ
```c
bgCost_t bgWayCost(const bgWay_t *way, const bgWayContext_t *wc) {
  bgCost_t c = 1;

  for(bgIdx_t i = 0; i < wc->dim; i++) {
    if(wc->weights[i] > 0) {
      c += way->indicators[i] * wc->weights[i];
    }else {
      c += (1 - way->indicators[i]) * -wc->weights[i];
    }
  }

  c /= wc->dim + 1;
  c *= way->length;
  return c;
}
```

$$
cost = \frac{length}{dim + 1} \left [ \sum_{i=1}^{dim}
\left\{ \begin{matrix}
\bm{indicator}_i \times \bm{weight}_i & (\bm{weight}_i \ge 0) \\
(1 - \bm{indicator}_i) \times (-\bm{weight}_i) & (\bm{weight}_i < 0) \\
\end{matrix} \right\}
+ 1 \right ]
$$

基本的にはノードコスト計算は同じです。
最後にウェイの長さでコストをスケールします。
また、すべての重みが0でも長さデータが失われないように、必ず1となる仮想の指標値を追加して計算します。

## 3. 指標と重みの意味
### (1) 重みの絶対値
重みが0に近いと、他の指標と比べて、コストへの影響が相対的に小さくなります。<br>
重みが0から離れていると、他の指標と比べて、コストへの影響が相対的に大きくなります。<br>

### (2) 重みの符号
重みが正であると、指標が大きいほどコストが大きくなります。<br>
重みが負であるとき、指標が小さいほどコストが大きくなります。<br>

### (3) コスト
コストが大きくなるとは、経路探索で避けられるということです。

## 4. ヒューリスティックコスト
```c
static bgCost_t heuristic(const bgNode_t *node, double goal_lon, double goal_lat) {
  bgCost_t manhattan_dist = fabs(goal_lon - node->lon) + fabs(goal_lat - node->lat);
  return manhattan_dist;
}
```
A\*のヒューリスティックコストには緯度経度のマンハッタン距離を用います。
マンハッタン距離を用いる理由は計算が楽だからです。

> [!CAUTION]
> grapher、beltgraph両方で距離には緯度経度の角度の距離を用いています。
> これは正しい距離計算ではないです。
> より広い地図を用いる場合や、緯度、経度の符号が変化する場合に問題になります。

## 5. 既知の問題
### (1) ノードとウェイのコストの割合
ウェイのコストは長さスケールを行うため、ノードのコストと比べて、大きい又は小さくなるという問題があります。
現状は緯度経度の長さスケールを行うため、ウェイのコストは非常に小さくなります。
この、ノードとウェイのコストの比は議論すべきです。

> 以前の実装では、指標値の正規化前に、長さスケールを行っていたため、この問題は発生していませんでした。
> しかし、以前の実装では重みがすべて0であるときに、扱いずらかったため、変更しました。

### (2) ヒューリスティックの割合
ノードとウェイのコストで、最後に次元で割る処理をしています。
これは、指標数が変わってもヒューリスティックの割合が保たれると考えて追加しました。
しかし、実際は入力重みによってヒューリスティックの割合が大きく変わり、探索時間の差が大きいです。

### (3) 非直感的
重みを大きくすると避けるというところが、よくなさそうです。

# 指標・コスト・重み Version 3
> [!IMPORTANT]
> 改良版を開発中です。

> [!NOTE]
> コスト関数は頻繁に呼び出されるため、できるだけgrapherで事前計算し、beltgraphで高速に計算できる方が好ましいです。
> 緯度経度距離を用いたり、平方根、三角関数をあまり使っていない理由でもあります。

$$
switch(\bm{indicator}_i) = \left\{ 
  \begin{matrix}
    (1 - \bm{indicator}_i) & (\bm{weight}_i \ge 0) \\
    \bm{indicator}_i & (\bm{weight}_i < 0) \\
  \end{matrix}
\right\}
$$

## ノード
$$
cost = \mu \sum_{i=1}^{k} |\bm{weight}_i \times switch(\bm{indicator}_i)|
$$
$$
\mu = 10^{-4}
$$

## ウェイ
$$
cost = length \left[
  1 + \sum_{i=1}^{l} |\bm{weight}_i \times switch(\bm{indicator}_i)|
\right]
$$

## ヒューリスティック
$$
heuristic = manhattan\_distance \left[
  1 +
  \sum_{i=1}^{k} \bm{node\_weight}_i^2 +
  \sum_{i=1}^{l} \bm{way\_weight}_i^2
\right]
$$
