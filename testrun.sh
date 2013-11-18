#!/bin/bash

program="./reduction"
runs=50
danger_threshold=105000000

function test_for_size ()
{
    size=$1
    stime=0
    echo -n "Testing size=${size}: " 1>&2
    for i in $(seq 1 $runs);
    do
        if [ $(($i % 10)) -eq 0 ]
        then
            echo -n "$i" 1>&2
        else
            echo -n '.' 1>&2
        fi
        cmd="$program $RANDOM $size"
        ctime=`$cmd | awk '/^Kernel/ { print \$3 }'`
        if [ "$ctime" == "" ] 
        then
            echo "Error occurred!: $cmd"
            exit
        fi
        stime=$(( $stime + $ctime ))
    done
    echo "" 1>&2
    mtime=$(echo "$stime / $runs" | bc)
    echo "$mtime; "
#    echo "size=$size mean_time=$mtime"

}

if [ "$1" != "" ] 
then
    program=$1
fi

for size in 1 10 100 1000 100000 1000000 10000000 105000000
do
    test_for_size $size
done

echo "" 1>&2
echo "===================================== DANGER ZONE =====================================" 1>&2
echo "" 1>&2
echo "WARNING: THE LATER TESTS INCLUDES SIZES GREATER THAN $danger_threshold ." 1>&2
echo "         THEY MIGHT CRASH THE GPU IF YOUR PROGRAM IS NOT WRITTEN WITH PROPER PROTECTION." 1>&2
echo "   " 1>&2
echo "         PRESS CTRL+C TO ABORT NOW OR ANY KEY TO CONTINUE" 1>&2
#read 1>&2

runs=10

for size in 110000000 500000000 1000000000 5000000000 10000000000 100000000000
do
    test_for_size $size
done


