#include "main.h"
#define SQLCMD_BUFF_SIZE 5000
#define INVALID_PRODUCT_ID 0

/**
 * @brief Inserts a new product into the database and retrieves its generated ID.
 *
 * This function handles the insertion of a new product for a specific supplier into the
 * `product` table in the database. It ensures that the supplier exists and generates
 * a unique product ID upon successful insertion. The function also provides feedback
 * about the operation's success or failure through the `msg` parameter.
 *
 * @param db               Database connection object used to execute SQL queries.
 * @param id_supplier      ID of the supplier associated with the new product.
 * @param product_quantity Initial quantity of the product to be added.
 * @param msg              Reference to a string used to store success or error messages.
 * 
 * @return The auto-generated ID of the newly inserted product, or `INVALID_PRODUCT_ID` on failure.
 */

unsigned long saveProductQuery(Con2DB db, const unsigned long id_supplier,
                               const unsigned int product_quantity,
                               std::string &msg){

  char sqlcmd[SQLCMD_BUFF_SIZE];
  PGresult *res;
  unsigned long product_id = INVALID_PRODUCT_ID; 
  
  //Checks whether the supplier id exists
  snprintf(sqlcmd, sizeof(sqlcmd), "SELECT NOT EXISTS(SELECT 1 FROM supplier WHERE id = %ld)", id_supplier);
  res = db.ExecSQLtuples(sqlcmd);
  
  if(PQgetvalue(res, 0, 0)[0] == 't'){
        msg = "Supplier not registered";
        PQclear(res);
        return INVALID_PRODUCT_ID;
  }

  memset(sqlcmd, '\0', sizeof(sqlcmd));
  PQclear(res); 
  
  //Inserts a new product and gets the generated ID
  snprintf(sqlcmd, sizeof(sqlcmd),
      "INSERT INTO product (supplier, quantity, price, datetime) "
      "VALUES (%ld, %d, NULL, current_timestamp) RETURNING id", id_supplier, product_quantity);
  
  res = db.ExecSQLtuples(sqlcmd);

  if(PQresultStatus(res) == PGRES_TUPLES_OK){
      product_id = std::stoul(PQgetvalue(res, 0, 0));
  } 
  else{
      msg = "Failed to insert product";
      PQclear(res);
      return INVALID_PRODUCT_ID;
  }
 
  memset(sqlcmd, '\0', sizeof(sqlcmd));
  PQclear(res);

  msg = "Product saved successfully";
  return product_id;  
}
