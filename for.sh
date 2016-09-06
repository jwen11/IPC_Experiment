#!/bin/bash

DEBUGFS=`grep debugfs /proc/mounts | awk '{ print $2; }'`

# echo function_graph > $DEBUGFS/tracing/current_tracer

for i in `seq 1 3`;
do
	echo function_graph > $DEBUGFS/tracing/current_tracer
	sudo ./producer 1 &
	sleep 1
	sudo ./consumer 1 
	sleep 1
	cp $DEBUGFS/tracing/trace trace$i
done

# for i in `seq 1 2`;
# do
#     echo $i
#     sudo ./producer 1 &
#     sleep 1
# 	sudo ./consumer 1 
# 	sleep 1
#     trace-cmd extract -o trace$i.dat
#     #cp $DEBUGFS/tracing/trace trace$i
# done   

