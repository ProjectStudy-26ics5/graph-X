# 指標・コスト・重み
grapherおよびbeltgraphの指標、コスト、重みについて講義します。

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

### ノード
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

> [!CAUTION]
> ウェイのコストは長さスケールを行うため、ノードのコストと比べて、大きい又は小さくなるという問題があります。
> 現状は緯度経度の長さスケールを行うため、ウェイのコストは非常に小さくなります。
> この、ノードとウェイのコストの比は議論すべきです。
>
> > 以前の実装では、指標値の正規化前に、長さスケールを行っていたため、この問題は発生していませんでした。
> > しかし、以前の実装では重みがすべて0であるときに、扱いずらかったため、変更しました。

## 3. 指標と重みの意味
### (1) 重みの絶対値
重みが0に近いと、他の指標と比べて、コストへの影響が相対的に小さくなります。<br>
重みが0から離れていると、他の指標と比べて、コストへの影響が相対的に大きくなります。<br>

### (2) 重みの符号
重みが正であると、指標が大きいほどコストが大きくなります。<br>
重みが負であるとき、指標が小さいほどコストが大きくなります。<br>

### (3) コスト
コストが大きくなるとは、経路探索で避けられるということです。
