#!/bin/bash

d=$1
orig=-$((d/2))
n=0

for ((i=0;i<$d;i++))
do for ((j=0;j<$d;j++))
  do
    if [ $i -ge $j ] ; then continue ; fi ;
    printf "location $n $((j+$orig)) $((-$orig-i)) "
    for ((k=0 ; k<$((d-2)) ; k++)) ; do printf "0 " ; done
    printf "\n" ;
    n=$((n+1))
  done
done
for ((i=0;i<$d;i++))
do for ((j=0;j<$d;j++))
  do
    if [ $i -le $j ] ; then continue ; fi ;
    printf "location $n $((j+$orig)) $((-$orig-i)) "
    for ((k=0 ; k<$((d-2)) ; k++)) ; do printf "0 " ; done
    printf "\n" ;
    n=$((n+1))
  done
done

nn=$((d-1))
nn=$((nn*d/2))
n=0
for ((i=0 ; i<$nn ; i++))
do
  printf "omega $n "
  for ((j=0;j<$i;j++)) ; do printf "0 " ; done
  printf "1 " ;
  for ((j=$((i+1)) ; j<$nn ; j++)) ; do printf "0 " ; done ;
  printf "\n"
  n=$((n+1)) ;
done

for ((i=0 ; i<$d ; i++))
do
    for ((j=0 ; j<i ; j++))
    do
        tmp=$((j*j+j))
        linidx=$((j*d-tmp/2+ i-1-j))
        printf "omega $n "
        for ((k=0;k<$linidx;k++)) ; do printf "0 " ; done
        printf "1 " ;
        for ((k=$((linidx+1)) ; k<$nn ; k++)) ; do printf "0 " ; done ;
        printf "\n"
        n=$((n+1)) ;
    done
done
