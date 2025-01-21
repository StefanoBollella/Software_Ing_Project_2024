#include "main.h"

#define SQLCMD_BUFF_SIZE 500

int cancelOrderQuery(
    Con2DB db,
    unsigned long custID,
    unsigned long orderID,
    std::string &msg
) {

    char *bool_res;
    char sqlcmd[SQLCMD_BUFF_SIZE]; 
    PGresult *res;

    // PRECONDITIONS
    // 1. EXISTS c, o Customer(c) && id(c, custID) && 
    //      Order(o) && custOrd(c, o) && id(o, orderID) &&
    //      not Traveling(o)

    snprintf(
        sqlcmd, sizeof(sqlcmd),
        "SELECT EXISTS("
            "SELECT 1 FROM customer c, order_t o "
            "WHERE c.id = %lu AND o.id = %lu "
            "AND o.customer = c.id"
        ") "
        "AND NOT EXISTS("
        "SELECT 1 FROM traveling t "
        "WHERE t.order_id = %lu"
        ")",
        custID, orderID, orderID
    );
    res = db.ExecSQLtuples(sqlcmd);
    memset(sqlcmd, '\0', sizeof(sqlcmd));

    bool_res = PQgetvalue(res, 0, 0);
    PQclear(res);
    if (*bool_res == 'f') {
       msg = "customer is not registered, order doesn't exists,"
             " or order is traveling";
       return EXIT_FAILURE;
    }

    snprintf(sqlcmd, sizeof(sqlcmd), "BEGIN");
    db.ExecSQLcmd(sqlcmd);
    memset(sqlcmd, '\0', sizeof(sqlcmd));

    // inserts a row to indicate that the order was cancelled
    snprintf(
        sqlcmd, sizeof(sqlcmd),
        "INSERT INTO canceled(order_id, datetime) "
        "VALUES (%lu, current_timestamp)",
        orderID
    );
    db.ExecSQLcmd(sqlcmd);
    memset(sqlcmd, '\0', sizeof(sqlcmd));

    // restock the products of the order that was cancelled
    snprintf(
        sqlcmd, sizeof(sqlcmd),
        "UPDATE product "
        "SET quantity = product.quantity + op.quantity "
        "FROM order_t o, ordprod op "
        "WHERE o.id = op.order_id "
        "AND product.id = op.product_id "
        "AND o.customer = %lu "
        "AND o.id = %lu",
        custID, orderID
    );
    db.ExecSQLcmd(sqlcmd);
    memset(sqlcmd, '\0', sizeof(sqlcmd));

    snprintf(sqlcmd, sizeof(sqlcmd), "COMMIT");
    db.ExecSQLcmd(sqlcmd);

    return EXIT_SUCCESS;
}
