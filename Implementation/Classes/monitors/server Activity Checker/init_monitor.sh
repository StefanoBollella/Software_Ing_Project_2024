#!/bin/bash
# Parameters:
# - pong_supplier: Redis stream for supplier server's PONG responses
# - pong_customer: Redis stream for customer server's PONG responses
# - pong_carrier: Redis stream for carrier server's PONG responses
# - ping_supplier: Redis stream for sending PING requests to the supplier server
# - ping_customer: Redis stream for sending PING requests to the customer server
# - ping_carrier: Redis stream for sending PING requests to the carrier server
# - 5000: Blocking time (in milliseconds) for waiting for PONG responses
echo "Esecuzione di init_monitor.sh..."

#Compile the monitor
echo "Compilazione del monitor serverActivityChecker..."
cd src && make clean && make && cd ..

./bin/monitor pong_supplier pong_customer pong_carrier ping_supplier ping_customer ping_carrier 5000 


