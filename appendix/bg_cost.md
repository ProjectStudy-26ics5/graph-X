# beltgraphの経路探索のコスト計算
コスト($ cost $)は、コンテキストの重み($ \bm{weight} $)と、指標値($ \bm{indicator} $)を用いて計算されます。
ノードとウェイで、同じ名称を利用していますが別のものです。

## 制約
$$
-1 \le \bm{weight}_i \le 1 \\
0 \le \bm{indicator}_i \le 1 \\
0 \le cost \\
$$

## ノード
$$
cost = \frac{1}{dim} \sum_{i=1}^{dim}
\left\{ \begin{matrix}
\bm{indicator}_i \times \bm{weight}_i & (\bm{weight}_i \ge 0) \\
(1 - \bm{indicator}_i) \times (-\bm{weight}_i) & (\bm{weight}_i < 0) \\
\end{matrix} \right\}
$$

指標値と重みを掛け合わせる形になります。もし、重みが負なら、指標を反転します。
最後に指標、重みの次元で割ります。

## ウェイ
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

## 指標と重みの意味
### (1) 重みの絶対値
重みが0に近いと、他の指標と比べて、コストへの影響が相対的に小さくなります。<br>
重みが0から離れていると、他の指標と比べて、コストへの影響が相対的に大きくなります。<br>

### (2) 重みの符号
重みが正であると、指標が大きいほどコストが大きくなります。<br>
重みが負であるとき、指標が小さいほどコストが大きくなります。<br>

### (3) コスト
コストが大きくなるとは、経路探索で避けられるということです。
