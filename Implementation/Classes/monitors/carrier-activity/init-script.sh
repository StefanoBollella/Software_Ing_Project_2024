#!/bin/bash

echo "Compiling carrier-activity monitor..."
cd src && make clean && make && cd ..

# 1 000 000 microseconds = 1000 milliseconds = 1 second
./bin/carrier-activity 10000000 2 1 # exec; 10 seconds; x operations(threshold); count_get_orders = 1 (true)
