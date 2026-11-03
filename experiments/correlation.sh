#!/usr/bin/bash

#相関係数用データ集め自動化スクリプト
#全指標についてwaystepweight，nodestepweightを実行し，それぞれ別ファイルに保存します。

export BG_FILE="bg/minato.bg"
NAMES_WAY=(
  "width" 
  "surface" 
  "lanes" 
  "cycle_lane" 
  "oneway" 
  "maxspeed" 
  "height" 
  "surface_detail" 
  "lit" 
  "barrier" 
  "tracktype" 
  "sac_scale" 
  "sidewalk"
)
NAMES_NODE=(
  "rank" 
  "traffic_signal"
)

for i in {0..12}
do
    export RESULT_FILE="waystepweight_${NAMES_WAY[$i]}.plt"
    sudo -E task e-waystepweight <<< $i
done

for i in {0..1}
do
    export RESULT_FILE="nodestepweight_${NAMES_NODE[$i]}.plt"
    sudo -E task e-nodestepweight <<< $i
done