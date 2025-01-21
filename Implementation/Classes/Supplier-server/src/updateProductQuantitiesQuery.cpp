#include "main.h"
#define SQLCMD_BUFF_SIZE 5000

/**
 * @brief Updates the quantities of products for a specific supplier in the database.
 *
 * This function processes an update request by modifying the quantities of products
 * associated with a given supplier. It ensures that all updates are performed within
 * a transaction to maintain consistency. If any update fails, the transaction is rolled
 * back, and the operation is terminated with an error message.
 *
 * @param db Database connection object used to execute SQL commands.
 * @param id_supplier ID of the supplier whose products are being updated.
 * @param products A map containing product IDs as keys and the quantities to be added as values.
 * @param msg A reference to a string where error or success messages will be stored.
 * 
 * @return EXIT_SUCCESS if all product quantities are updated successfully, otherwise EXIT_FAILURE.
 */

int updateProductQuantitiesQuery(Con2DB db, const unsigned long id_supplier, 
                                 std::unordered_map<unsigned long, unsigned int> products, 
                                 std::string &msg){
    char sqlcmd[SQLCMD_BUFF_SIZE];
    char BEGIN[250];
    char ROLLBACK[250];
    char COMMIT[250];
    PGresult *res;
   
    //Checks whether the supplier ID exists
    snprintf(sqlcmd, sizeof(sqlcmd), "SELECT NOT EXISTS(SELECT 1 FROM supplier WHERE id = %ld)", id_supplier);
    res = db.ExecSQLtuples(sqlcmd);
    char* bool_res = PQgetvalue(res, 0, 0);
    
    if(bool_res[0] == 't'){
        msg = "Supplier not registered";
        PQclear(res);
        return EXIT_FAILURE;
    }

    PQclear(res); 

    //Start a transaction to ensure that all queries are executed correctly
    snprintf(BEGIN, sizeof(BEGIN), "BEGIN");
    db.ExecSQLcmd(BEGIN);

    //It runs through the unordered_map of products and updates the corresponding quantities in the database
    for(const auto& product : products){
        unsigned long product_id = product.first;  
        unsigned int quantity = product.second;    

        snprintf(sqlcmd, sizeof(sqlcmd), "UPDATE product SET quantity = quantity + %u WHERE id = %lu AND supplier = %lu", 
                 quantity, product_id, id_supplier);

        res = db.ExecSQLcmd(sqlcmd);

       
        if(PQresultStatus(res) != PGRES_COMMAND_OK){
            msg = "Failed to update product with id: " + std::to_string(product_id);
            PQclear(res);
            
             //Takes a rollback in case of an error
             snprintf(ROLLBACK, sizeof(ROLLBACK), "ROLLBACK");
             db.ExecSQLcmd(ROLLBACK);
            return EXIT_FAILURE;
        }

        PQclear(res);  
    }

     //If all queries execute successfully, commit the transaction
     snprintf(COMMIT, sizeof(COMMIT), "COMMIT");
     db.ExecSQLcmd(COMMIT);

    msg = "Product quantities updated successfully.";
    return EXIT_SUCCESS;
}

