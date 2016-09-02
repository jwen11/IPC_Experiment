#! /bin/bash

for i in $(ls ..) ;
do
    if [ ${i} != "util" ];
    then
        echo ${i}
        python ./collect_data.py ${i}
    fi
done
