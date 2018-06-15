#!/bin/bash

CORES=$(cat /proc/cpuinfo | awk '/^processor/{print $3}' | tail -1);
PRECISION=$1;
if [ -z $PRECISION ]
then
  PRECISION=1000;
fi

for ((i=1; i <= $CORES + 1; i++))
do
  echo $(./project -t $i -p $PRECISION -q -stat);
done
