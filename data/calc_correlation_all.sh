#!/usr/bin/bash

PROGRAM_DIR="$(pwd)/experiments"
SOURCE_DIR="$(pwd)/data/${1}"
export BG_FILE="data/${1}/graph.bg"

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
if [ -e ${SOURCE_DIR}/correlation.txt ]; then
  rm ${SOURCE_DIR}/experiments/correlation.txt
fi
cc ${PROGRAM_DIR}/correlation.c -o ${PROGRAM_DIR}/correlation -lm
if [ ! -e ${SOURCE_DIR}/experiments/waystepweight ]; then
  mkdir ${SOURCE_DIR}/experiments/waystepweight
fi
if [ ! -e ${SOURCE_DIR}/experiments/nodestepweight ]; then
  mkdir ${SOURCE_DIR}/experiments/nodestepweight
fi

for i in {0..12}
do
  export RESULT_DIR="${SOURCE_DIR}/experiments/waystepweight/${NAMES_WAY[$i]}"
  if [ ! -e ${SOURCE_DIR}/experiments/waystepweight/${NAMES_WAY[$i]} ]; then
    mkdir ${SOURCE_DIR}/experiments/waystepweight/${NAMES_WAY[$i]}
  fi
  if [ -e ${SOURCE_DIR}/experiments/waystepweight/${NAMES_WAY[$i]}/waystepweight.plt ]; then
    rm ${SOURCE_DIR}/experiments/waystepweight/${NAMES_WAY[$i]}/waystepweight.plt
  fi
  touch ${SOURCE_DIR}/experiments/waystepweight/${NAMES_WAY[$i]}/waystepweight.plt
  ./build/experiments/waystepweight <<< $i
  echo ${NAMES_WAY[$i]} >> ${SOURCE_DIR}/experiments/correlation.txt
  ${PROGRAM_DIR}/correlation ${SOURCE_DIR}/experiments/waystepweight/${NAMES_WAY[$i]}/waystepweight.plt >> ${SOURCE_DIR}/experiments/correlation.txt
  echo -e >> ${SOURCE_DIR}/experiments/correlation.txt
  echo -e
done

for i in {0..1}
do
  export RESULT_DIR="${SOURCE_DIR}/experiments/nodestepweight/${NAMES_NODE[$i]}"
  if [ ! -e ${SOURCE_DIR}/experiments/nodestepweight/${NAMES_NODE[$i]} ]; then
    mkdir ${SOURCE_DIR}/experiments/nodestepweight/${NAMES_NODE[$i]}
  fi
  if [ -e ${SOURCE_DIR}/experiments/nodestepweight/${NAMES_NODE[$i]}/nodestepweight.plt ]; then
    rm ${SOURCE_DIR}/experiments/nodestepweight/${NAMES_NODE[$i]}/nodestepweight.plt
  fi
  touch ${SOURCE_DIR}/experiments/nodestepweight/${NAMES_NODE[$i]}/nodestepweight.plt
  echo ${NAMES_NODE[$i]} >> ${SOURCE_DIR}/experiments/correlation.txt
  ./build/experiments/nodestepweight <<< $i
  ${PROGRAM_DIR}/correlation ${SOURCE_DIR}/experiments/nodestepweight/${NAMES_NODE[$i]}/nodestepweight.plt >> ${SOURCE_DIR}/experiments/correlation.txt
  echo -e >> ${SOURCE_DIR}/experiments/correlation.txt
  echo -e
done