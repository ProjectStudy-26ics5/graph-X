#!/usr/bin/bash

# 相関係数用データ集め自動化スクリプト
# 全指標についてwaystepweight，nodestepweightを実行し，それぞれ別ファイルに保存します。

# 変数宣言
export BG_FILE="bg/minato.bg"
DIR="$(pwd)/experiments"
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

# init
if [ -e ${DIR}/correlation.txt ]; then
  rm ${DIR}/correlation.txt
fi
cc ${DIR}/correlation.c -o ${DIR}/correlation -lm

# waystepweight
for i in {0..12}
do
  if [ -e ${DIR}/waystepweight_${NAMES_WAY[$i]}.plt ]; then
    rm ${DIR}/waystepweight_${NAMES_WAY[$i]}.plt
  fi
  export RESULT_FILE="waystepweight_${NAMES_WAY[$i]}.plt"
  ./build/experiments/waystepweight <<< $i
  echo ${NAMES_WAY[$i]} >> ${DIR}/correlation.txt
  ./experiments/correlation ${DIR}/waystepweight_${NAMES_WAY[$i]}.plt >> ${DIR}/correlation.txt
  echo -e >> ${DIR}/correlation.txt
  echo -e
done

# nodestepweight
for i in {0..1}
do
  if [ -e ${DIR}/nodestepweight_${NAMES_NODE[$i]}.plt ]; then
      rm ${DIR}/nodestepweight_${NAMES_NODE[$i]}.plt
  fi
  export RESULT_FILE="nodestepweight_${NAMES_NODE[$i]}.plt"
  ./build/experiments/nodestepweight <<< $i
  echo ${NAMES_NODE[$i]} >> ${DIR}/correlation.txt
  ./experiments/correlation ${DIR}/nodestepweight_${NAMES_NODE[$i]}.plt >> ${DIR}/correlation.txt
  echo -e >> ${DIR}/correlation.txt
  echo -e
done

echo -e