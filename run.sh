#! /bin/bash
sudo ./producer 1 &
sleep 1
sudo ./consumer 1 
sleep 1

sudo ./producer 1024 &
sleep 1
sudo ./consumer 1024 
sleep 1

sudo ./producer 4096 &
sleep 1
sudo ./consumer 4096 
sleep 1

sudo ./producer 8192 &
sleep 1
sudo ./consumer 8192
sleep 1

sudo ./producer 16384 &
sleep 1
sudo ./consumer 16384
sleep 1

sudo ./producer 32768 &
sleep 1
sudo ./consumer 32768
sleep 1

sudo ./producer 65536 &
sleep 1
sudo ./consumer 65536
sleep 1
