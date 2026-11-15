
import json
import csv
from pathlib import Path

import matplotlib
matplotlib.use("Agg")

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.collections import LineCollection

BASE = Path(__file__).resolve().parent

# --------------------------------------------------
# データ読み込み
# --------------------------------------------------

with open(BASE / "network.json", encoding="utf-8") as f:
    network = json.load(f)

with open(BASE / "paths.json", encoding="utf-8") as f:
    paths = json.load(f)["paths"]

with open(BASE / "metrics.csv", encoding="utf-8") as f:
    metrics = list(csv.DictReader(f))

indicators = ["width", "rank", "surface", "traffic_signal"]

labels = {
    "width": "Road width",
    "rank": "Node rank",
    "surface": "Road surface",
    "traffic_signal": "Traffic signal",
}

# --------------------------------------------------
# 道路ネットワークの座標処理
# edge = [lat1, lon1, lat2, lon2]
# --------------------------------------------------

edges = np.asarray(network["edges"], dtype=float)

if edges.size == 0:
    raise ValueError("network.json に道路データがありません")

all_lats = edges[:, [0, 2]].ravel()
all_lons = edges[:, [1, 3]].ravel()

center_lat = np.mean(all_lats)
aspect = 1.0 / np.cos(np.radians(center_lat))

# LineCollection用の座標
segments = [
    [(edge[1], edge[0]), (edge[3], edge[2])]
    for edge in edges
]

print(f"Road edges: {len(edges)}")
print(f"Route paths: {len(paths)}")

# --------------------------------------------------
# 指標ごとの経路比較図
# --------------------------------------------------

for indicator in indicators:
    fig, ax = plt.subplots(figsize=(9, 9))

    # 道路ネットワーク
    ax.add_collection(
        LineCollection(
            segments,
            colors="lightgray",
            linewidths=0.35,
            alpha=0.45,
            zorder=1,
        )
    )

    selected = [
        p for p in paths
        if p["indicator"] == indicator
    ]

        # 重みごとの経路
    for p in sorted(selected, key=lambda x: x["weight"]):
        coords = np.asarray(p["coordinates"], dtype=float)

        if coords.size == 0:
            continue

        weight = p["weight"]

        if weight == 0:
            color = "black"
            linewidth = 2.5
            alpha = 1.0
            zorder = 5
        elif weight == 1:
            color = "red"
            linewidth = 2.5
            alpha = 1.0
            zorder = 6
        else:
            color = plt.cm.viridis(weight)
            linewidth = 1.0
            alpha = 0.35
            zorder = 3

        ax.plot(
            coords[:, 1],
            coords[:, 0],
            color=color,
            linewidth=linewidth,
            alpha=alpha,
            zorder=zorder,
        )

# --------------------------------------------------
# 重みと経路長の関係
# --------------------------------------------------

fig, axes = plt.subplots(2, 2, figsize=(11, 8))

for ax, indicator in zip(axes.flat, indicators):
    rows = [
        r for r in metrics
        if r["indicator"] == indicator
    ]

    weights = [float(r["weight"]) for r in rows]
    lengths = [float(r["path_length_m"]) for r in rows]

    ax.plot(weights, lengths, marker="o")
    ax.set_title(labels[indicator])
    ax.set_xlabel("Weight")
    ax.set_ylabel("Path length (stored units)")
    ax.set_xticks(np.arange(0, 1.01, 0.1))
    ax.grid(alpha=0.3)

fig.suptitle("Relationship between weight and path length")
fig.tight_layout()
fig.savefig(BASE / "metrics.png", dpi=300)
plt.close(fig)

# --------------------------------------------------
# 重みと経路上の指標平均値の関係
# --------------------------------------------------

fig, axes = plt.subplots(2, 2, figsize=(11, 8))

for ax, indicator in zip(axes.flat, indicators):
    rows = [
        r for r in metrics
        if r["indicator"] == indicator
    ]

    weights = [float(r["weight"]) for r in rows]
    means = [float(r["indicator_mean"]) for r in rows]

    ax.plot(weights, means, marker="o")
    ax.set_title(labels[indicator])
    ax.set_xlabel("Weight")
    ax.set_ylabel("Mean indicator value")
    ax.set_xticks(np.arange(0, 1.01, 0.1))
    ax.grid(alpha=0.3)

fig.suptitle("Relationship between weight and indicator value")
fig.tight_layout()
fig.savefig(BASE / "indicator_means.png", dpi=300)
plt.close(fig)

print("\nAll figures generated:")
for p in sorted(BASE.glob("*.png")):
    print(p.name)
