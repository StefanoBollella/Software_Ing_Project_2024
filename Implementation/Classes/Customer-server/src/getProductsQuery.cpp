#include "main.h"

#define SQLCMD_BUFF_SIZE 5000

int getProductsQuery(
    Con2DB db, unsigned numProds, std::string &msg,
    std::vector<std::pair<unsigned long, unsigned long>> &products
) {

    char sqlcmd[SQLCMD_BUFF_SIZE];
    int nTuples;
    unsigned long id, qty;
    PGresult *res;
    
    // PRECONDITIONS: nothing
    
    snprintf(
        sqlcmd, sizeof(sqlcmd),
        "SELECT id, quantity FROM product LIMIT %u",
        numProds
    );
    res = db.ExecSQLtuples(sqlcmd);

    nTuples = PQntuples(res);
    if (nTuples == 0) {
        msg = "out of stocks"; 
        return EXIT_FAILURE;
    }
    memset(sqlcmd, '\0', sizeof(sqlcmd));

    // reads the query result
    for (int i = 0; i < nTuples; ++i) {
        id = std::stoul(PQgetvalue(res, i, PQfnumber(res, "id")));
        qty = std::stoul(PQgetvalue(res, i, PQfnumber(res, "quantity")));
        products.emplace_back(std::make_pair(id, qty));
    }

    return EXIT_SUCCESS;

}
