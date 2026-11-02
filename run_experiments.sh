#!/bin/bash

regions=(minato takao nerima edogawa hachijo)
experiments=(
  bgplot
  nodeindplot
  nodestepweight
  pathplot
  pathtime
  wayindplot
  waystepweight
)

for region in "${regions[@]}"; do
  echo "========== $region =========="

  mkdir -p "data/$region/experiments"

  export BG_FILE="data/$region/graph.bg"
  export RESULT_DIR="data/$region/experiments"

  for exp in "${experiments[@]}"; do
    echo "---- $exp ----"
    ./build-wsl/experiments/"$exp" || {
      echo "ERROR: $region / $exp"
      exit 1
    }
  done
done

echo "ALL EXPERIMENTS COMPLETED"
