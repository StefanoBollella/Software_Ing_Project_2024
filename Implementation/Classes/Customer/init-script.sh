#!/bin/bash

echo "Compiling Customer..."
# Cleans the object files and then compiles the program
cd ./src/ && make clean && make && cd ..

# Executes the customer
echo "Executing customer test generator..."
echo "Copying stdout and stderr to stdout and log.txt..."
./bin/customer src/customer_config.yaml req_stream reply_stream |& tee -a log.txt
