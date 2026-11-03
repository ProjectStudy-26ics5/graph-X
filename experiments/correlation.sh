#!/usr/bin/bash
export BG_FILE="bg/minato.bg"
for i in {1..10}
do
    echo "Running experiment $i"
    export RESULT_FILE="waystepweight_$i.plt"
    sudo -E task e-waystepweight <<< $i
done