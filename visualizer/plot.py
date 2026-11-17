import json
import csv
from pathlib import Path
from collections import defaultdict

import matplotlib
matplotlib.use("Agg")

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.collections import LineCollection
from matplotlib.lines import Line2D
from matplotlib.colors import Normalize

BASE = Path(__file__).resolve().parent

indicators = ["width", "rank", "traffic_signal"]

labels = {
    "width": "Road width",
    "rank": "Node rank",
    "traffic_signal": "Traffic signal",
}

# --------------------------------------------------
# データ読み込み
# --------------------------------------------------

with open(BASE / "network.json", encoding="utf-8") as f:
    network = json.load(f)

with open(BASE / "paths.json", encoding="utf-8") as f:
    paths = json.load(f)["paths"]

with open(BASE / "metrics.csv", encoding="utf-8") as f:
    metrics = list(csv.DictReader(f))

edges = np.asarray(network["edges"], dtype=float)

if edges.size == 0:
    raise ValueError("network.json に道路データがありません")

print(f"Road edges: {len(edges)}")
print(f"Route paths: {len(paths)}")

all_lats = edges[:, [0, 2]].ravel()
all_lons = edges[:, [1, 3]].ravel()

center_lat = np.mean(all_lats)
aspect = 1.0 / np.cos(np.radians(center_lat))

lon_min, lon_max = np.min(all_lons), np.max(all_lons)
lat_min, lat_max = np.min(all_lats), np.max(all_lats)

lon_margin = max((lon_max - lon_min) * 0.03, 0.0001)
lat_margin = max((lat_max - lat_min) * 0.03, 0.0001)

def edge_key(a, b):
    """始点・終点の順序に依存しない道路区間キー"""
    a = (round(float(a[0]), 7), round(float(a[1]), 7))
    b = (round(float(b[0]), 7), round(float(b[1]), 7))
    return tuple(sorted((a, b)))

# --------------------------------------------------
# 経路比較図：重複区間を一度だけ描画
# --------------------------------------------------

for indicator in indicators:

    selected = [p for p in paths if p["indicator"] == indicator]

    # 道路区間ごとに、通過した経路の重みを集約
    edge_weights = defaultdict(list)

    for p in selected:
        coords = np.asarray(p["coordinates"], dtype=float)

        if len(coords) < 2:
            continue

        weight = float(p["weight"])

        for i in range(len(coords) - 1):
            a = coords[i]
            b = coords[i + 1]
            key = edge_key(a, b)
            edge_weights[key].append(weight)

    unique_segments = []
    mean_weights = []

    for key, weights in edge_weights.items():
        a, b = key
        unique_segments.append([
            (a[1], a[0]),
            (b[1], b[0])
        ])
        mean_weights.append(np.mean(weights))

    fig, ax = plt.subplots(figsize=(10, 10))

    # 背景道路
    ax.add_collection(
        LineCollection(
            [
                [(edge[1], edge[0]), (edge[3], edge[2])]
                for edge in edges
            ],
            colors="lightgray",
            linewidths=0.35,
            alpha=0.5,
            zorder=1,
        )
    )

    # 重複を除いた経路区間
    if unique_segments:
        collection = LineCollection(
            unique_segments,
            cmap="coolwarm",
            norm=Normalize(vmin=-1, vmax=1),
            linewidths=2.0,
            alpha=0.95,
            zorder=4,
        )
        collection.set_array(np.asarray(mean_weights))
        ax.add_collection(collection)

        cbar = fig.colorbar(collection, ax=ax, fraction=0.035, pad=0.02)
        cbar.set_label("Mean weight of routes using the segment")

    ax.set_xlim(lon_min - lon_margin, lon_max + lon_margin)
    ax.set_ylim(lat_min - lat_margin, lat_max + lat_margin)
    ax.set_aspect(aspect)

    ax.set_title(
        f"Route comparison: {labels[indicator]}\n"
        f"Unique route segments: {len(unique_segments)}",
        fontsize=14,
    )
    ax.set_xlabel("Longitude")
    ax.set_ylabel("Latitude")
    ax.grid(alpha=0.2)

    fig.tight_layout()
    output = BASE / f"route_{indicator}.png"
    fig.savefig(output, dpi=300, bbox_inches="tight")
    plt.close(fig)

    print(f"Generated: {output.name} "
          f"({len(unique_segments)} unique segments)")

# --------------------------------------------------
# 指標値分布ヒートマップ
# network.json に edge_indicators がある場合に生成
# --------------------------------------------------

edge_indicators = network.get("edge_indicators")

if edge_indicators is None:
    print(
        "\nHeatmaps skipped: network.json に edge_indicators がありません。"
        "\nmain.cpp 側で道路区間ごとの指標値を出力する必要があります。"
    )
else:
    for indicator in indicators:

        values = np.asarray(
            [item[indicator] for item in edge_indicators],
            dtype=float
        )

        valid = np.isfinite(values)

        if not np.any(valid):
            print(f"Skipped heatmap: {indicator} (no valid values)")
            continue

        heat_segments = [
            [(edge[1], edge[0]), (edge[3], edge[2])]
            for edge, ok in zip(edges, valid)
            if ok
        ]

        fig, ax = plt.subplots(figsize=(10, 10))

        collection = LineCollection(
            heat_segments,
            cmap="viridis",
            linewidths=1.4,
            norm=Normalize(
                vmin=np.min(values[valid]),
                vmax=np.max(values[valid])
            ),
            zorder=2,
        )
        collection.set_array(values[valid])
        ax.add_collection(collection)

        cbar = fig.colorbar(collection, ax=ax, fraction=0.035, pad=0.02)
        cbar.set_label(labels[indicator])

        ax.set_xlim(lon_min - lon_margin, lon_max + lon_margin)
        ax.set_ylim(lat_min - lat_margin, lat_max + lat_margin)
        ax.set_aspect(aspect)
        ax.set_title(f"Indicator distribution: {labels[indicator]}")
        ax.set_xlabel("Longitude")
        ax.set_ylabel("Latitude")
        ax.grid(alpha=0.2)

        fig.tight_layout()
        output = BASE / f"heatmap_{indicator}.png"
        fig.savefig(output, dpi=300, bbox_inches="tight")
        plt.close(fig)

        print(f"Generated: {output.name}")

# --------------------------------------------------
# 重みと経路長の関係
# --------------------------------------------------

fig, axes = plt.subplots(2, 2, figsize=(11, 8))

for ax, indicator in zip(axes.flat, indicators):
    rows = [r for r in metrics if r["indicator"] == indicator]
    rows.sort(key=lambda r: float(r["weight"]))

    weights = [float(r["weight"]) for r in rows]
    lengths = [float(r["path_length_m"]) for r in rows]

    ax.plot(weights, lengths, linewidth=1.0)
    ax.set_title(labels[indicator])
    ax.set_xlabel("Weight")
    ax.set_ylabel("Path length (m)")
    ax.set_xticks([-1, -0.5, 0, 0.5, 1])
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
    rows = [r for r in metrics if r["indicator"] == indicator]
    rows.sort(key=lambda r: float(r["weight"]))

    weights = [float(r["weight"]) for r in rows]
    means = [float(r["indicator_mean"]) for r in rows]

    ax.plot(weights, means, linewidth=1.0)
    ax.set_title(labels[indicator])
    ax.set_xlabel("Weight")
    ax.set_ylabel("Mean indicator value")
    ax.set_xticks([-1, -0.5, 0, 0.5, 1])
    ax.grid(alpha=0.3)

fig.suptitle("Relationship between weight and indicator value")
fig.tight_layout()
fig.savefig(BASE / "indicator_means.png", dpi=300)
plt.close(fig)

print("\nAll figures generated:")
for p in sorted(BASE.glob("*.png")):
    print(p.name)
