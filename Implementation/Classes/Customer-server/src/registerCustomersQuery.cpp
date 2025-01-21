#include "main.h"

#define SQLCMD_BUFF_SIZE 100

std::vector<unsigned long> registerCustomersQuery(Con2DB db, int qty) {

    char sqlcmd[SQLCMD_BUFF_SIZE];
    PGresult* res;
    std::vector<unsigned long> customerIDs;

    snprintf(sqlcmd, sizeof(sqlcmd), "BEGIN");
    db.ExecSQLcmd(sqlcmd);
    memset(sqlcmd, '\0', sizeof(sqlcmd));

    snprintf(
        sqlcmd,
        sizeof(sqlcmd),
        "INSERT INTO customer(spawntime) "
        "VALUES (current_timestamp) RETURNING id"
    );

    while (qty) {
        res = db.ExecSQLtuples(sqlcmd);
#if 0
        if (PQntuples(res) == 0) {
            // TODO: Add a msg reference string to pass to the response
            // of the handler
            memset(sqlcmd, '\0', sizeof(sqlcmd));
            snprintf(sqlcmd, sizeof(sqlcmd), "ROLLBACK"); 
            db.ExecSQLcmd(sqlcmd);
            //TODO: Add what to return to the handler after ROLLBACK
            // (no tuples inserted)
        }
#endif
        customerIDs.push_back(
            std::stoul(PQgetvalue(res, 0, PQfnumber(res, "id")))
        );
        PQclear(res);
        --qty;
    }

    memset(sqlcmd, '\0', SQLCMD_BUFF_SIZE);
    snprintf(sqlcmd, SQLCMD_BUFF_SIZE, "COMMIT");
    db.ExecSQLcmd(sqlcmd);

    return customerIDs;
}
