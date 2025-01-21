#!/bin/bash

declare -A stati_diz
declare -i base=0
declare -i ceiling=1
#!/bin/bash

# File di output
OUTPUT_FILE="result2.csv"
HEADER="count,state,start,end"

if [ ! -f "$OUTPUT_FILE" ]; then
    echo "$HEADER" > "$OUTPUT_FILE"
fi 

UNIT="minute"

DB_HOST="localhost"
DB_PORT="5432"
DB_USER="ecommerce_db_usr"
DB_NAME="ecommerce_db"
DB_PASSWORD="47002"


# loop
for ((i = 0; i < 60; i=i+1)); do
    QUERY_BASE_CEILING="
        with bc as (
            select datetime + ${base} * interval '0.5 ${UNIT}' as base,
                   datetime + ${ceiling} * interval '0.5 ${UNIT}' as ceiling,
                   ${base} * interval '0.5 ${UNIT}' as b,
                   ${ceiling} * interval '0.5 ${UNIT}' as c,
                   0 as fake
            from order_t order by datetime asc limit 1
        )"

    stati_diz[pending]="
        ${QUERY_BASE_CEILING}
        select coalesce(t.count, 0), 'pending' as state, bc.b as start, bc.c as end
        from bc 
        left join
        (
            select count(*), 0 as fake
            from order_t, bc
            where (order_t.datetime >= bc.base) and (order_t.datetime < bc.ceiling) 
            and (order_t.id not in (select order_id from traveling tr where tr.datetime >= bc.base and tr.datetime < bc.ceiling)) 
            and (order_t.id not in (select order_id from canceled where canceled.datetime >= bc.base and canceled.datetime < bc.ceiling))
        ) t
        ON bc.fake = t.fake"

    stati_diz[canceled]="
        ${QUERY_BASE_CEILING}
        select coalesce(t.count, 0), 'canceled' as state, bc.b as start, bc.c as end
        from bc
        left join
        (
            select count(*), 0 as fake 
            from canceled c, bc
            where (c.datetime >= bc.base) and (c.datetime < bc.ceiling) 
        ) t 
        ON bc.fake = t.fake"

    stati_diz[traveling]="
        ${QUERY_BASE_CEILING}
        select coalesce(t.count, 0), 'traveling' as state, bc.b as start, bc.c as end
        from bc 
        left join
        (
            select count(*), 0 as fake
            from traveling tr, bc
            where (tr.datetime >= bc.base) and (tr.datetime < bc.ceiling) 
            and (tr.order_id not in (select order_id from delivered dv where dv.datetime >= bc.base and dv.datetime < bc.ceiling)) 
            and (tr.order_id not in (select order_id from lost where lost.datetime >= bc.base and lost.datetime < bc.ceiling)) 
        ) t 
        ON bc.fake = t.fake"

    stati_diz[delivered]="
        ${QUERY_BASE_CEILING}
        select coalesce(t.count, 0), 'delivered' as state, bc.b as start, bc.c as end
        from bc 
        left join
        (
            select count(*), 0 as fake
            from delivered dv, bc
            where (dv.datetime >= bc.base) and (dv.datetime < bc.ceiling) 
        ) t 
        ON bc.fake = t.fake"

    stati_diz[lost]="
        ${QUERY_BASE_CEILING}
        select coalesce(t.count, 0), 'lost' as state, bc.b as start, bc.c as end
        from bc 
        left join
        (
            select count(*), 0 as fake
            from lost ls, bc
            where (ls.datetime >= bc.base) and (ls.datetime < bc.ceiling) 
        ) t 
        ON bc.fake = t.fake"

for key in "${!stati_diz[@]}"; do 
    #echo "${stati_diz[$key]}"
    PGPASSWORD=$DB_PASSWORD psql -h $DB_HOST -p $DB_PORT -U $DB_USER -d $DB_NAME -c "\COPY (${stati_diz[$key]}) TO STDOUT WITH (FORMAT csv, HEADER false)" >> ${OUTPUT_FILE}
done
    echo "${base} ${ceiling}"
#base=$(( base + 1 ))
ceiling=$(( ceiling + 1 ))
done
