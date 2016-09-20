##sudo rm /tmp/myFIFO
##make clean
##make P4080=true

testset="`seq 1 8` 16 32 64"
#testset="64 16 32"

if [ -e "log/"*"log" ]; then
    echo "Please make sure the log directory contains no irrelevant logs"
    exit
elif [ -d "log/log"* ]; then
    echo "Please make sure the log directory contains no irrelevant log directories"
    exit
fi 

for i in ${testset};
do
    echo ${i};
done


sudo killall bandwidth
for i in ${testset};
do
    sudo ./producer ${i} r &
    sleep 1
    sudo ./consumer ${i} r
    sleep 4
done    

mkdir log/log_iso
mv log/*log log/log_iso



sudo ~jwen11/isolbench/run_inter.sh
for i in ${testset};
do
    sudo ./producer ${i} r &
    sleep 1
    sudo ./consumer ${i} r
    sleep 4
done    

sudo killall bandwidth
mkdir log/log_iin_r
mv log/*log log/log_iin_r

sudo ~jwen11/isolbench/run_inter_w.sh
for i in ${testset};
do
    sudo ./producer ${i} w &
    sleep 1
    sudo ./consumer ${i} w
    sleep 4
done    

sudo killall bandwidth
mkdir log/log_iin_w
mv log/*log log/log_iin_w


sudo killall bandwidth
for i in ${testset};
do
    sudo ./producer ${i} n &
    sleep 1
    sudo ./consumer ${i} n
    sleep 4
done    

mkdir log/log_niso
mv log/*log log/log_niso



sudo ~jwen11/isolbench/run_inter.sh
for i in ${testset};
do
    sudo ./producer ${i} n &
    sleep 1
    sudo ./consumer ${i} n
    sleep 4
done    

sudo killall bandwidth
mkdir log/log_niin_r
mv log/*log log/log_niin_r

sudo ~jwen11/isolbench/run_inter_w.sh
for i in ${testset};
do
    sudo ./producer ${i} n &
    sleep 1
    sudo ./consumer ${i} n
    sleep 4
done    

sudo killall bandwidth
mkdir log/log_niin_w
mv log/*log log/log_niin_w
