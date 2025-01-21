#!/bin/bash

# Initializes the DB for the server
# cd ../ecommerce-db-scripts && ./create.sh && cd ../Customer-server

# Initializes the DB for the logs
# cd ../log-db-scripts && ./create.sh && cd ../Customer-server

echo "Compiling Customer-Server..."
# Cleans the object files and then compiles the program
cd ./src/ && make clean && make && cd ..

# Executes the first instance of the server
./bin/server req_stream reply_stream 5000 ping_customer pong_customer 
