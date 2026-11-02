#!/bin/bash
set -uo pipefail

regions=(minato takao nerima edogawa hachijo)

experiments=(
  nodeindplot
  nodestepweight
  pathplot
  pathtime
  wayindplot
  waystepweight
)

for region in "${regions[@]}"; do
  echo "========== $region : BG generation =========="

  mkdir -p "data/$region/experiments"

  if ! ./build-wsl/tests/converter "$region"; then
    echo "[ERROR] converter failed: $region"
    continue
  fi

  export BG_FILE="data/$region/graph.bg"
  export RESULT_DIR="data/$region/experiments"

  echo "========== $region : experiments =========="

  for exp in "${experiments[@]}"; do
    echo "---- $exp ----"

    if "./build-wsl/experiments/$exp"; then
      echo "[OK] $region / $exp"
    else
      echo "[ERROR] $region / $exp"
    fi
  done
done

unset BG_FILE RESULT_DIR
echo "ALL REGIONS PROCESSED"
