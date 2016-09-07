#!/bin/bash

DEBUGFS=`grep debugfs /proc/mounts | awk '{ print $2; }'`
data="1"
# echo function_graph > $DEBUGFS/tracing/current_tracer
echo "data size is:$data kb"
touch ./log/producer_$data.log
# sudo bash -c 'echo "producer_$data" > ./log/producer_$data.log'
echo "" > ./log/producer_$data.log

for i in `seq 1 5`;
do
	echo function_graph > $DEBUGFS/tracing/current_tracer
	sudo ./producer $data &
	sleep 1
	sudo ./consumer $data 
	sleep 1
	cp $DEBUGFS/tracing/trace trace$i
	echo nop > $DEBUGFS/tracing/current_tracer
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

