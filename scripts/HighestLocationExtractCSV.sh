#!/bin/bash

ls dump-*.csv | sort -k 1.5 -nr | xargs -n 1 awk 'BEGIN{max=-100000;FS=","} NR>1{if ($1>max) max=$1} END{print max}'

