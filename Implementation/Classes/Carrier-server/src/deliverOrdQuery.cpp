#include "main.h"

#define SQLCMD_BUFF_SIZE 5000

int deliverOrdQuery( Con2DB db, unsigned long carrierID, unsigned long orderID, std::string &msg )   {

    char *bool_res;
    char sqlcmd[SQLCMD_BUFF_SIZE];
    PGresult *res;

    // first check that the preconditions to the query exist: the order still exists and is just in traveling
    // there must be the same orderID and carrierID in table TRAVELING and it is not delivered nor lost
    snprintf(
        sqlcmd, sizeof(sqlcmd),
        "SELECT EXISTS( "
            "SELECT * "
            "FROM TRAVELING tr "
            "WHERE tr.ORDER_ID = %lu AND tr.CARRIER_ID = %lu) ", 
        orderID, carrierID
    );

    res = db.ExecSQLtuples(sqlcmd);
    memset(sqlcmd, '\0', sizeof(sqlcmd));

    bool_res = PQgetvalue(res, 0, 0);
    PQclear(res);
    if(bool_res[0] == 'f'){
        msg = "the order with the specific carrier was not found traveling";
        return EXIT_FAILURE;
    }

    // checking if the order is already delivered or already lost
    snprintf(
        sqlcmd, sizeof(sqlcmd),
        "SELECT EXISTS( "
            "SELECT * "
            "FROM DELIVERED dv, LOST ls "
            "WHERE (dv.ORDER_ID = %lu AND dv.CARRIER_ID = %lu) OR (ls.ORDER_ID = %lu AND ls.CARRIER_ID = %lu))", 
        orderID, carrierID
    );

    res = db.ExecSQLtuples(sqlcmd);
    memset(sqlcmd, '\0', sizeof(sqlcmd));

    bool_res = PQgetvalue(res, 0, 0);
    PQclear(res);
    if(bool_res[0] == 't'){
        msg = "the order is already delivered or lost, something went wrong...";
        return EXIT_FAILURE;
    }    

    // second execute the query

    snprintf(
        sqlcmd, sizeof(sqlcmd),
        "INSERT INTO DELIVERED(ORDER_ID, DATETIME, CARRIER_ID) "
        "VALUES (%lu, current_timestamp, %lu)", 
        orderID, carrierID
    );

    db.ExecSQLcmd(sqlcmd);
    memset(sqlcmd, '\0', sizeof(sqlcmd));

    /*
    bool_res = PQgetvalue(res, 0, 0);
    PQclear(res);
    if(bool_res == 't'){
        msg = "the order is already delivered or lost, something went wrong..."
        return EXIT_FAILURE;
    }    */ 

    // check the query returned with success, if so go on: there is no reason to check, so just go on


    return EXIT_SUCCESS;
}