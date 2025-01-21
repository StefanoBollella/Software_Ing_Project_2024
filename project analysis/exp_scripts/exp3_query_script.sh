#!/bin/bash
#QUERY: calcola per ciascun ordine il tempo impiegato dalla creazione alla consegna 
# File di output
OUTPUT_FILE="result3.csv"
HEADER="id,travel_time"

if [ ! -f "$OUTPUT_FILE" ]; then
    echo "$HEADER" > "$OUTPUT_FILE"
fi 

DB_HOST="localhost"
DB_PORT="5432"
DB_USER="ecommerce_db_usr"
DB_NAME="ecommerce_db"
DB_PASSWORD="47002"

QUERY="SELECT ord.id, dv.datetime - ord.datetime AS travel_time 
       FROM delivered dv, order_t ord
       WHERE dv.order_id = ord.id"

# Esegui la query e gestisci eventuali errori
if ! PGPASSWORD=$DB_PASSWORD psql -h $DB_HOST -p $DB_PORT -U $DB_USER -d $DB_NAME \
    -c "\COPY ($QUERY) TO STDOUT WITH (FORMAT csv, HEADER false)" >> "$OUTPUT_FILE"; then
    echo "Errore durante l'esecuzione della query o la scrittura del file."
    exit 1
fi

