#!/bin/bash
start=`date +%s`
for i in $(seq 1 $2); do
    cp $1 files/$1$i;
done
end=`date +%s`

echo $(($end-$start))
