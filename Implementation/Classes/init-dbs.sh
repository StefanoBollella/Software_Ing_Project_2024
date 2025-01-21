#!/bin/bash

# Initializes the DB for the server
cd ./ecommerce-db-scripts && ./create.sh

# Initializes the DB for the logs
cd ../log-db-scripts && ./create.sh
