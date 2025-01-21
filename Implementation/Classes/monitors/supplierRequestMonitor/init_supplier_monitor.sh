#!/bin/bash

echo "Running init_supplier_monitor.sh..."

INTERVAL_MICROSECONDS=10000000 #Interval in microseconds for the monitor
MIN_REQUEST=3 #minimum number of requests possible in the INTERVAL_MICROSECONDS interval 

echo "Compiling monitors..."
cd src && make clean && make && cd ..

echo "Starting the functional monitor supplierRequestMonitor"
./bin/supplier_monitor $INTERVAL_MICROSECONDS $MIN_REQUEST

