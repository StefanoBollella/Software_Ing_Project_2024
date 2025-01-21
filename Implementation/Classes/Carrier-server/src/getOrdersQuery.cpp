#include "main.h"

#define SQLCMD_BUFF_SIZE 5000

std::vector<unsigned long> getOrdersQuery(Con2DB db, int ordsNum, std::string &msg, int *funcRes, unsigned long carrierID){

    // use: there are orders in order_t THAT ARE NOT IN TRAVELING, and you move them to traveling

    std::vector<unsigned long> ordersID;
    char sqlcmd[SQLCMD_BUFF_SIZE];
    int nTuples;
    unsigned long id;
    PGresult *res;
    
    // query:
    snprintf(
        sqlcmd, sizeof(sqlcmd),
        "SELECT ID "
        "FROM ORDER_T "
        "WHERE ID NOT IN ( "
            "SELECT ORDER_ID "
            "FROM TRAVELING) "
            "AND  ID NOT IN ( "
            "SELECT ORDER_ID "
            "FROM CANCELED) "
        "LIMIT %u ",
        ordsNum
    );
    res = db.ExecSQLtuples(sqlcmd);

    nTuples = PQntuples(res);
    if (nTuples == 0) {
        msg = "no orders were found waiting for pickup"; 
        *funcRes = EXIT_FAILURE;
        return ordersID;
    }
    memset(sqlcmd, '\0', sizeof(sqlcmd));

    // reads the query result
    for (int i = 0; i < nTuples; ++i) {
        id = std::stoul(PQgetvalue(res, i, PQfnumber(res, "id")));
        ordersID.emplace_back(id);
    }

    // now the taken orders are placed in the traveling table

	for(unsigned long ordID:ordersID){
		
        snprintf(
            sqlcmd, sizeof(sqlcmd),
            "INSERT INTO TRAVELING(ORDER_ID, DATETIME, CARRIER_ID) "
            "VALUES (%lu, current_timestamp, %lu)",
            ordID,carrierID
        );
        db.ExecSQLcmd(sqlcmd);
        memset(sqlcmd, '\0', sizeof(sqlcmd));  

	}   

    *funcRes = EXIT_SUCCESS;
    return ordersID;

    
}