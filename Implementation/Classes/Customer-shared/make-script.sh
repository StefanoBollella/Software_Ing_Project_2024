#!/bin/bash

echo "Compiling Customer-shared..."
cd src && make clean && make && cd .. 
