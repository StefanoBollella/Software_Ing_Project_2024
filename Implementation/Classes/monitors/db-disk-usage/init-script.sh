#!/bin/bash

echo "Compiling db-disk-usage monitor"
cd src && make clean && make && cd ..

# Input is the max number of MB that each DB can be allocated
./bin/db-disk-usage-monitor 8 
