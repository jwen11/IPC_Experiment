#! /bin/bash

sudo rm /tmp/myFIFO
make clean
make 



for i in `seq 1 8`;
do
    sudo ./producer ${i} r &
    sleep 1
    sudo ./consumer ${i} r
    sleep 4
    sudo rm /tmp/myFIFO
done    

mkdir log/log_iso
mv log/*log log/log_iso
