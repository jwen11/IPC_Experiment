#! /bin/bash

#compute the mean, 99th %, ... from the raw data
for i in $(ls ..) ;
do
    if [ ${i} != "util" ];
    then
        echo ${i}
        python ./collect_data.py ${i}
        sort -n ${i}.result -o ${i}.result
    fi
done

#paste all the mean in a same file
if [ -e "summary.txt" ];
then
    rm -f ./summary.txt
fi
python ./merge.py
