#!/bin/bash

runs=50

for size in 1 2 5 10 100 1000 100000 1000000 10000000 105000000
do
    stime=0
    echo -n "Testing size=${size}: " 1>&2
    for i in $(seq 1 $runs);
    do
        if [ $(($i % 10)) -eq 0 ]
        then
            echo -n "$i" 1>&2
        else
            echo -n '.'
        fi
        cmd="./reduction $RANDOM $size"
        ctime=`$cmd | awk '/^Kernel/ { print \$3 }'`
        if [ "$ctime" == "" ] 
        then
            echo "Error occurred!: $cmd"
            exit
        fi
        stime=$(( $stime + $ctime ))
    done
    echo ""
    mtime=$(echo "$stime / $runs" | bc)
#    echo "$size,$mtime"
    echo "size=$size mean_time=$mtime"
done
