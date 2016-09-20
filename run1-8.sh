#! /bin/bash

make clean
make P4080=true
for i in `seq 1 8`;
do
    sudo ./producer ${i} r &
    sudo ./consumer ${i} r
    sleep 1
done    
