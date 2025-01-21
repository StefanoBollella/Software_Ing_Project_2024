#include "main.h"

#define SQLCMD_BUFF_SIZE 5000

using namespace std;

int orderQuery(
    Con2DB db,
    unsigned long custID,
    unsigned long *ordID,
    vector<pair<unsigned long, unsigned long>> &products,
    std::string &msg
) {

    char *bool_res;
    char sqlcmd[SQLCMD_BUFF_SIZE];
    char valuesClauseBuf[SQLCMD_BUFF_SIZE];
    std::vector<std::string> tuples;
    std::string strRepTuples;
    PGresult *res;

    // PRECONDITIONS
    /* Check if customerID exists
     * Check if each product exists
     * Check if each product in the order has sufficient qty for the order.
     */
    snprintf(
        sqlcmd, sizeof(sqlcmd),
        "SELECT NOT EXISTS (SELECT 1 FROM customer WHERE id = %lu)",
        custID
    );
    res = db.ExecSQLtuples(sqlcmd);
    bool_res = PQgetvalue(res, 0, 0);
    if (bool_res[0] == 't') {
        msg = "customer not registered";
        return EXIT_FAILURE;
    }
    memset(sqlcmd, '\0', sizeof(sqlcmd));
    PQclear(res);

    strRepTuples = vecTuplesToStr(products);
    snprintf(
        sqlcmd, sizeof(sqlcmd),
        "WITH order_data AS " 
        "(SELECT * FROM (VALUES %s) AS temp(id, qty)) "
        "SELECT order_data.id, order_data.qty, COALESCE(product.quantity, 0) "
        "FROM order_data "
        "LEFT JOIN product on order_data.id = product.id "
        "WHERE product.id IS NULL "
        "OR product.quantity < qty",
        strRepTuples.substr(1, strRepTuples.length() - 2).c_str()
    );
    res = db.ExecSQLtuples(sqlcmd);
    if (PQntuples(res) > 0) {
        msg = "some products are unavailable or don't have enough stocks";
        return EXIT_FAILURE;
    }
    memset(sqlcmd, '\0', sizeof(sqlcmd));
    PQclear(res);

    // BEGIN TRANSACTION
    snprintf(sqlcmd, sizeof(sqlcmd), "BEGIN");
    db.ExecSQLcmd(sqlcmd);
    memset(sqlcmd, '\0', sizeof(sqlcmd));

    snprintf(
        sqlcmd, sizeof(sqlcmd),
        "INSERT INTO order_t(datetime, customer) "
        "VALUES (current_timestamp, %lu) returning id",
        custID
    );
    res = db.ExecSQLtuples(sqlcmd);
    if (PQntuples(res) == 0) {
        msg = "failed to insert a new order";

        memset(sqlcmd, '\0', sizeof(sqlcmd));
        snprintf(sqlcmd, sizeof(sqlcmd), "ROLLBACK"); 
        db.ExecSQLcmd(sqlcmd);

        return EXIT_FAILURE;
    }
    // gets the id of the inserted order
    *ordID = std::stoul(PQgetvalue(res, 0, PQfnumber(res, "id")));
    memset(sqlcmd, '\0', sizeof(sqlcmd));
    PQclear(res);

    // creates the tuples (orderId, prodID, qty) for the VALUES clause of INSERT
    for (std::pair<unsigned long, unsigned long> p: products) {
        tuples.push_back(
            std::string("(") + std::to_string(*ordID) + ", " +
            std::to_string(p.first) + ", " +
            std::to_string(p.second) + ")"
        );
    }
    sqlValuesClause(valuesClauseBuf, sizeof(valuesClauseBuf), tuples);

    if (snprintf(sqlcmd, sizeof(sqlcmd),
        "INSERT INTO ordprod(order_id, product_id, quantity) %s",
        valuesClauseBuf) >= (long)sizeof(sqlcmd)) {
        msg = "buffer overflow occured: values clause for sql insert";

        memset(sqlcmd, '\0', sizeof(sqlcmd));
        snprintf(sqlcmd, sizeof(sqlcmd), "ROLLBACK"); 
        db.ExecSQLcmd(sqlcmd);

        return EXIT_FAILURE;
    }
    db.ExecSQLcmd(sqlcmd);
    memset(sqlcmd, '\0', sizeof(sqlcmd));
    memset(valuesClauseBuf, '\0', sizeof(valuesClauseBuf));

    // creates tempt table for bulk update of product qtys
    snprintf(
        sqlcmd, 
        sizeof(sqlcmd),
        "CREATE TEMP TABLE update_table("
        "id INTEGER NOT NULL PRIMARY KEY, "
        "taken INTEGER NOT NULL) "
        "ON COMMIT DROP"
    );
    db.ExecSQLcmd(sqlcmd);

    tuples.clear();
    // creates the tuples (id, taken) for the VALUES clause of INSERT
    for (std::pair<unsigned long, unsigned long> p: products) {
        tuples.push_back(
            std::string("(") + std::to_string(p.first) + ", " +
            std::to_string(p.second) + ")"
        );
    }
    sqlValuesClause(valuesClauseBuf, sizeof(valuesClauseBuf), tuples);

    // inserts the new values into the temp table
    if (snprintf(sqlcmd, sizeof(sqlcmd),
        "INSERT INTO update_table(id, taken) %s",
        valuesClauseBuf) >= (long)sizeof(sqlcmd)) {
        msg = "buffer overflaw occured: values clause for sql insert";

        memset(sqlcmd, '\0', sizeof(sqlcmd));
        snprintf(sqlcmd, sizeof(sqlcmd), "ROLLBACK"); 
        db.ExecSQLcmd(sqlcmd);

        return EXIT_FAILURE;
    }
    db.ExecSQLcmd(sqlcmd);
    memset(sqlcmd, '\0', sizeof(sqlcmd));

    // updates the product table with the values of the temp table
    snprintf(
        sqlcmd, sizeof(sqlcmd),
        "UPDATE product "
        "SET quantity = quantity - u.taken "
        "FROM update_table u "
        "WHERE product.id = u.id"
    );
    db.ExecSQLcmd(sqlcmd);
    memset(sqlcmd, '\0', sizeof(sqlcmd));

    snprintf(sqlcmd, sizeof(sqlcmd), "COMMIT");
    db.ExecSQLcmd(sqlcmd);

    return EXIT_SUCCESS; 
}
