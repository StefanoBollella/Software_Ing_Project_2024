#include "main.h"
#define SQLCMD_BUFF_SIZE 5000

/**
 * @brief Registers a specified number of new suppliers in the database and retrieves their IDs.
 *
 * This function inserts a given quantity of new suppliers into the `supplier` table of the database.
 * Each supplier is assigned a unique ID generated automatically by the database, which is retrieved
 * and stored in a vector for further processing.
 *
 * @param db               Database connection object for executing SQL queries.
 * @param supplier_quantity The number of new suppliers to register in the database.
 * @param msg              Reference to a string that stores error messages, if any.
 * 
 * @return A vector of `unsigned long` containing the IDs of the newly registered suppliers. 
 *         If an error occurs, the vector will be empty.
 */

std::vector<unsigned long> registerSuppliersQuery(Con2DB db, const int supplier_quantity, 
                                                 std::string &msg){
   
std::vector<unsigned long> supplierIDs;
char sqlcmd[SQLCMD_BUFF_SIZE];
PGresult *res;

    snprintf(sqlcmd, sizeof(sqlcmd), "BEGIN");
    db.ExecSQLcmd(sqlcmd);
    memset(sqlcmd, '\0', sizeof(sqlcmd));

    //Insert new suppliers and return their IDs
    for(int i = 0; i < supplier_quantity; ++i){
        snprintf(sqlcmd, sizeof(sqlcmd), 
        "INSERT INTO supplier(SPAWNTIME) "
        "VALUES (current_timestamp) RETURNING ID");

        res = db.ExecSQLtuples(sqlcmd);
        
        if (PQresultStatus(res) != PGRES_TUPLES_OK){
            msg = "Error inserting new supplier.";
            PQclear(res);
            return supplierIDs; 
        }

        supplierIDs.push_back(std::stoul(PQgetvalue(res, 0, PQfnumber(res, "ID"))));
        PQclear(res);
    }

    memset(sqlcmd, '\0', sizeof(sqlcmd));
    snprintf(sqlcmd, sizeof(sqlcmd), "COMMIT");
    db.ExecSQLcmd(sqlcmd);

 return supplierIDs;
}

