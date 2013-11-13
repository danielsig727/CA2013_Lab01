#!/bin/bash

runs=50

for size in 1 2 5 10 100 1000 100000 1000000 10000000 105000000
do
    stime=0
    for i in $(seq 1 $runs);
    do
        echo "Run $i/$runs of size $size..." 1>&2
        ctime=$(./reduction $RANDOM $size | awk '/^Kernel/ { print $3 }')
        if [ "$ctime" == "" ] 
        then
            echo "Error occurred!"
            exit
        fi
        stime=$(( $stime + $ctime ))
    done
    mtime=$(echo "$stime / $runs" | bc)
#    echo "$size,$mtime"
    echo "size=$size mean_time=$mtime"
done
