#!/bin/bash

echo "Compilazione di supplier_server..."
cd ./src && make clean && make
 
cd ../bin && ./supplier_server stream1 stream2 5000 pong_supplier ping_supplier && cd ..

