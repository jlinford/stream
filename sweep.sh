#!/bin/bash

#1
OMP_NUM_THREADS=1 KMP_AFFINITY=verbose OMP_PROC_BIND=spread OMP_PLACES={0} ./stream_c.exe

#2
OMP_NUM_THREADS=2 KMP_AFFINITY=verbose OMP_PROC_BIND=spread OMP_PLACES={0},{28} ./stream_c.exe

#4
OMP_NUM_THREADS=4 KMP_AFFINITY=verbose OMP_PROC_BIND=spread OMP_PLACES={0},{14},{28},{42} ./stream_c.exe

#8
OMP_NUM_THREADS=8 KMP_AFFINITY=verbose OMP_PROC_BIND=spread OMP_PLACES={0},{7},{14},{21},{28},{35},{42},{49} ./stream_c.exe

#12
OMP_NUM_THREADS=12 KMP_AFFINITY=verbose OMP_PROC_BIND=spread OMP_PLACES={0},{5},{10},{15},{20},{25},{28},{33},{38},{42},{47},{52} ./stream_c.exe

#16
OMP_NUM_THREADS=16 KMP_AFFINITY=verbose OMP_PROC_BIND=spread OMP_PLACES={0},{3},{7},{10},{14},{17},{21},{24},{28},{31},{35},{38},{42},{45},{49},{52} ./stream_c.exe

#20
OMP_NUM_THREADS=20 KMP_AFFINITY=verbose OMP_PROC_BIND=spread OMP_PLACES={0},{3},{6},{9},{12},{14},{17},{20},{23},{26},{28},{31},{34},{37},{40},{42},{45},{48},{51},{54} ./stream_c.exe

#24
OMP_NUM_THREADS=24 KMP_AFFINITY=verbose OMP_PROC_BIND=spread OMP_PLACES={0},{2},{4},{6},{8},{10},{12},{14},{16},{18},{20},{22},{24},{28},{30},{32},{34},{36},{38},{42},{44},{46},{48},{50} ./stream_c.exe

#28
OMP_NUM_THREADS=28 KMP_AFFINITY=verbose OMP_PROC_BIND=spread OMP_PLACES={0},{2},{4},{6},{8},{10},{12},{14},{16},{18},{20},{22},{24},{26},{28},{30},{32},{34},{36},{38},{40},{42},{44},{46},{48},{50},{52},{54} ./stream_c.exe

#32
OMP_NUM_THREADS=32 KMP_AFFINITY=verbose OMP_PROC_BIND=spread OMP_PLACES={0},{2},{4},{6},{7},{9},{11},{13},{14},{16},{18},{20},{21},{23},{25},{27},{28},{30},{32},{34},{35},{37},{39},{41},{42},{44},{46},{48},{49},{51},{53},{55} ./stream_c.exe

#40
OMP_NUM_THREADS=40 KMP_AFFINITY=verbose OMP_PROC_BIND=spread OMP_PLACES={0},{1},{2},{3},{4},{7},{8},{9},{10},{11},{14},{15},{16},{17},{18},{21},{22},{23},{24},{25},{28},{29},{30},{31},{32},{35},{36},{37},{38},{39},{42},{43},{44},{45},{46},{49},{50},{51},{52},{53} ./stream_c.exe

#44
OMP_NUM_THREADS=44 KMP_AFFINITY=verbose OMP_PROC_BIND=spread OMP_PLACES={0},{1},{2},{3},{4},{7},{8},{9},{10},{11},{12},{14},{15},{16},{17},{18},{21},{22},{23},{24},{25},{26},{28},{29},{30},{31},{32},{35},{36},{37},{38},{39},{40},{42},{43},{44},{45},{46},{49},{50},{51},{52},{53},{54} ./stream_c.exe

#48
OMP_NUM_THREADS=48 KMP_AFFINITY=verbose OMP_PROC_BIND=spread OMP_PLACES={0},{1},{2},{3},{4},{5},{7},{8},{9},{10},{11},{12},{14},{15},{16},{17},{18},{19},{21},{22},{23},{24},{25},{26},{28},{29},{30},{31},{32},{33},{35},{36},{37},{38},{39},{40},{42},{43},{44},{45},{46},{47},{49},{50},{51},{52},{53},{54} ./stream_c.exe

#52
OMP_NUM_THREADS=52 KMP_AFFINITY=verbose OMP_PROC_BIND=spread OMP_PLACES={0},{1},{2},{3},{4},{5},{6},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18},{19},{20},{22},{23},{24},{25},{26},{27},{28},{29},{30},{31},{32},{33},{34},{36},{37},{38},{39},{40},{41},{42},{43},{44},{45},{46},{47},{48},{50},{51},{52},{53},{54},{55} ./stream_c.exe

#56
OMP_NUM_THREADS=56 KMP_AFFINITY=verbose OMP_PROC_BIND=spread OMP_PLACES={0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18},{19},{20},{21},{22},{23},{24},{25},{26},{27},{28},{29},{30},{31},{32},{33},{34},{35},{36},{37},{38},{39},{40},{41},{42},{43},{44},{45},{46},{47},{48},{49},{50},{51},{52},{53},{54},{55} ./stream_c.exe

