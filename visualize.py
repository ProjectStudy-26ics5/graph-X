from pathlib import Path
import json
import pandas as pd
import matplotlib.pyplot as plt

REGION = "tamachi"
BASE_DIR = Path(__file__).resolve().parent
DATA_DIR = BASE_DIR / "output" / REGION
OUTPUT_DIR = DATA_DIR / "figures"
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

INDICATORS = ["width", "rank", "traffic_signal"]
WEIGHTS = [-1, -0.5, 0, 0.5, 1]

WEST, SOUTH = 139.735, 35.635
EAST, NORTH = 139.780, 35.692

with open(DATA_DIR / "network.json", encoding="utf-8") as f:
    network = json.load(f)

with open(DATA_DIR / "paths.json", encoding="utf-8") as f:
    paths = json.load(f)

metrics = pd.read_csv(DATA_DIR / "metrics.csv")

# 経路図
fig, axes = plt.subplots(1, 3, figsize=(21, 8))

for ax, indicator in zip(axes, INDICATORS):
    for edge in network["edges"]:
        lat1, lon1, lat2, lon2 = edge
        ax.plot(
            [lon1, lon2], [lat1, lat2],
            color="lightgray", linewidth=0.35
        )

    for path in paths["paths"]:
        if path["indicator"] != indicator:
            continue

        coords = path["coordinates"]
        if not coords:
            continue

        lats = [p[0] for p in coords]
        lons = [p[1] for p in coords]

        ax.plot(
            lons, lats,
            linewidth=1.8,
            label=f'weight={path["weight"]}'
        )

    ax.set_title(indicator)
    ax.set_xlim(WEST, EAST)
    ax.set_ylim(SOUTH, NORTH)
    ax.set_xlabel("Longitude")
    ax.set_ylabel("Latitude")
    ax.set_aspect(1 / 0.813)
    ax.grid(alpha=0.2)
    ax.legend(fontsize=8)

plt.tight_layout()
plt.savefig(OUTPUT_DIR / "routes.png", dpi=300)
plt.close()

# 評価グラフ
for indicator in INDICATORS:
    subset = metrics[metrics["indicator"] == indicator].copy()
    subset["weight"] = pd.to_numeric(subset["weight"])
    subset = subset.sort_values("weight")

    fig, ax1 = plt.subplots(figsize=(8, 5))

    ax1.plot(
        subset["weight"], subset["path_length_m"],
        marker="o", label="Path length"
    )
    ax1.set_xlabel("Weight")
    ax1.set_ylabel("Path length (m)")
    ax1.set_xticks(WEIGHTS)
    ax1.grid(alpha=0.3)

    ax2 = ax1.twinx()
    ax2.plot(
        subset["weight"], subset["indicator_mean"],
        marker="s", linestyle="--", label="Indicator mean"
    )
    ax2.set_ylabel("Indicator mean")

    lines1, labels1 = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax1.legend(lines1 + lines2, labels1 + labels2)

    plt.title(f"{indicator}: route evaluation")
    plt.tight_layout()
    plt.savefig(
        OUTPUT_DIR / f"metrics_{indicator}.png",
        dpi=300
    )
    plt.close()

print(f"Saved figures to: {OUTPUT_DIR}")
