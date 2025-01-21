#!/bin/bash

#Compila supplier_shared
echo "Compilazione di shared..."
cd ./src && make clean && make && cd ..
