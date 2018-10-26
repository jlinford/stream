#!/bin/bash

i=16
while [ $i -lt 34 ] ; do
  ((STREAM_ARRAY_SIZE=2**$i))
  make clean stream_c.exe STREAM_ARRAY_SIZE=$STREAM_ARRAY_SIZE
  OMP_NUM_THREADS=32 OMP_PROC_BIND=spread OMP_PLACES={0},{2},{4},{6},{7},{9},{11},{13},{14},{16},{18},{20},{21},{23},{25},{27},{28},{30},{32},{34},{35},{37},{39},{41},{42},{44},{46},{48},{49},{51},{53},{55} ./stream_c.exe 
  ((i++))
done
