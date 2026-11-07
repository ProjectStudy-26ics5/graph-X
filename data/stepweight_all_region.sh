#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."

run() {
    local region="$1"
    local type="$2"
    local index="$3"
    local indicator="$4"
    local program="./build/experiments/${type}stepweight"
    local filename="${type}_stepweight_${type^}_${indicator}.plt"

    echo "===== $region / $filename ====="

    REGION="$region" \
    BG_FILE="data/$region/graph.bg" \
    RESULT_DIR="data/$region/experiments" \
    RESULT_FILE="$filename" \
    "$program" < <(yes "$index")
}

for region in minato hachijo edogawa nerima takao; do
    [[ -f "data/$region/graph.bg" ]] || {
        echo "ERROR: data/$region/graph.bg not found" >&2
        exit 1
    }

    mkdir -p "data/$region/experiments"

    run "$region" way 0 width
    run "$region" way 1 surface
    run "$region" node 0 rank
    run "$region" node 1 traffic_signal
done

echo "All experiments completed."
