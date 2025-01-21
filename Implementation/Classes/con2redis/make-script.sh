#!/bin/bash

echo "Compilazione di con2redis..."
cd ./src && make clean && make && cd ..
