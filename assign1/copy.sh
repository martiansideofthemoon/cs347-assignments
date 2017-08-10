#!/bin/bash
start=`date +%s%N`
for i in $(seq 1 $2); do
    cp $1 files/$1$i;
done
end=`date +%s%N`

bc <<< 'scale=3; ('$end'-'$start')/1000000000'
