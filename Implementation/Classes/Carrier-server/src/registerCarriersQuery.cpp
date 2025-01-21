#include "main.h"

#define SQLCMD_BUFF_SIZE 200

std::vector<unsigned long> registerCarriersQuery(Con2DB db, int quantity){

    std::vector<unsigned long> carrierIDs;
    char sqlcmd[SQLCMD_BUFF_SIZE];
    PGresult *res;

    snprintf(sqlcmd, sizeof(sqlcmd), "BEGIN");

    db.ExecSQLcmd(sqlcmd);
    memset(sqlcmd, '\0', sizeof(sqlcmd));

    snprintf(sqlcmd, sizeof(sqlcmd), 
    "INSERT INTO CARRIER(SPAWNTIME) "
    "VALUES (current_timestamp) RETURNING ID ");

    while(quantity){
        res = db.ExecSQLtuples(sqlcmd);
        carrierIDs.push_back(
            std::stoul(PQgetvalue(res, 0, PQfnumber(res, "ID")))
        );
        PQclear(res);
        --quantity;
    }

    memset(sqlcmd, '\0', sizeof(sqlcmd));
    snprintf(sqlcmd, SQLCMD_BUFF_SIZE, "COMMIT");
    db.ExecSQLcmd(sqlcmd);

    return carrierIDs;
}