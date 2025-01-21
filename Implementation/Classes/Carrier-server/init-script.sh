#hello

#!/bin/bash

# Initializes the DB for the server
# cd ../ecommerce-db-scripts && ./create.sh && cd ../Carrier-server

# Initializes the DB for the logs
# cd ../log-db-scripts && ./create.sh && cd ../Carrier-server

echo "Compiling Carrier-Server..."
# Cleans the object files and then compiles the program
cd ./src/ && make clean && make && cd ..

# Executes the first instance of the server
cd ../Carrier-server && ./bin/server req_stream_main reply_stream_main 5000 pong_carrier ping_carrier 
server_pid1=$!
# echo "Started first server with PID $server_pid1"

# Executes the second instance of the server
#./bin/server req_stream_sub reply_stream_sub 5000 &
#server_pid2=$!
#echo "Started second server with PID $server_pid2"

## remember to start postgres: sudo systemctl start postgresql | only if necessary, now it should work
