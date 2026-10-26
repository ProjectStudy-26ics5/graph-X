# grapherの指標計算方法
`beltgraph/README.md`で説明した指標値計算は、正確には指標となる前の評価値($ \bm{E} $)の計算です。
指標値($ \bm{indicator} $)はこれをさらに加工します。その過程を以下に示します。

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
