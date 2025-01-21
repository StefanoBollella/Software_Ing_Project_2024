#!/bin/bash

echo "Compiling ord-server-proc-time monitor..."
cd src && make clean && make && cd ..
# 10,000,000 ns = 10,000 ms 
./bin/ord-server-proc-time-monitor 10000000
