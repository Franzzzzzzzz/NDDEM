#!/bin/bash


make debug && ./DEMND 3 1 $1 &
lmp_3p8p0 -in ../Liggghts/$1 && awk 'NF==5{print $0}' dump.myforce > Res1 &


