#!/bin/bash
SUPPLIER_CONFIG="supplier_config.yaml" 

echo "Esecuzione di init_supplier.sh..."

cd ./src && make clean && make

cd ../bin && ./supplier ../src/$SUPPLIER_CONFIG stream1 stream2 5000  && cd ..
