#include "main.h"
#define SQLCMD_BUFF_SIZE 5000

/**
 * @brief Retrieves the IDs of products with the minimum quantity for a specific supplier.
 * 
 * This function queries the database to find all product IDs associated with a given supplier (`id_supplier`) 
 * that have the same minimum quantity. It performs the following steps:
 * 
 * @param db An instance of `Con2DB`, used to execute SQL queries.
 * @param id_supplier The ID of the supplier whose product quantities are to be analyzed.
 * @param msg A reference to a `std::string` that is updated with an appropriate message 
 *            indicating the result of the query or any errors encountered.
 * @return A vector of unsigned long integers containing the IDs of the products with the 
 *         minimum quantity for the given supplier. If no products are found, the vector 
 *         is returned empty.
 */

std::vector<unsigned long> infoCurrentProductQuantitiesQuery(Con2DB db, const unsigned long id_supplier, std::string &msg){

  std::vector<unsigned long> product_ids;
  char * bool_res;
  char sqlcmd[SQLCMD_BUFF_SIZE];
  PGresult *res; 
  
  //Checks whether the supplier ID exists
  snprintf(sqlcmd, sizeof(sqlcmd), "SELECT NOT EXISTS(SELECT 1 FROM supplier WHERE id = %ld)", id_supplier);
  res = db.ExecSQLtuples(sqlcmd);
  bool_res = PQgetvalue(res, 0, 0);
  
  if(bool_res[0] == 't'){
        msg = "Supplier not registered";
        PQclear(res);
        return product_ids; 
  }

  memset(sqlcmd, '\0', sizeof(sqlcmd));
  PQclear(res);

  //Finds the currently recorded minimum quantity of products at id_supplier
  unsigned int min_quantity = 0;
  snprintf(sqlcmd, sizeof(sqlcmd),
           "SELECT MIN(quantity) FROM product WHERE supplier = %ld", id_supplier);

  res = db.ExecSQLtuples(sqlcmd);

  if (PQntuples(res) == 0 || PQgetvalue(res, 0, 0) == nullptr) {
        msg = "No products found for the given supplier.";
        PQclear(res);
        return product_ids;  // Nessun prodotto trovato
  }

  //Assigns the minimum quantity found to the `min_quantity` variable.
  min_quantity = std::stoul(PQgetvalue(res, 0, 0));
  PQclear(res);

  //Finds all IDs with the same 'min_quantity' as 'id_supplier'.
  snprintf(sqlcmd, sizeof(sqlcmd),
  "SELECT id FROM product WHERE supplier = %ld AND quantity = %u", id_supplier, min_quantity);

  res = db.ExecSQLtuples(sqlcmd);

  if (PQntuples(res) == 0) {
        msg = "No products found with the lowest quantity for the given supplier.";
        PQclear(res);
        return product_ids;  // Nessun prodotto trovato
  }

  //Adds the IDs of the products found to the vector
  for(int i = 0; i < PQntuples(res); ++i){
        unsigned long product_id = std::stoul(PQgetvalue(res, i, 0));
        product_ids.push_back(product_id);
  }

  PQclear(res);
  msg = "Product IDs with the lowest quantity retrieved successfully";
  return product_ids;
}

