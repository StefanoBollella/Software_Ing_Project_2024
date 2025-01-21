#!/bin/bash

echo "Compiling Supplier..."
# Cleans the object files and then compiles the program
cd ./src/ && make clean && make && cd ..

# Executes the carrier
./bin/carrier src/carrier_config.yaml req_stream_main reply_stream_main

#req_stream_sub reply_stream_sub
#carrier_pid=$!
#echo "Started carrier with PID $carrier_pid"
