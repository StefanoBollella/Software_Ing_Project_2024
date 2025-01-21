#include "main.h"
#define INDEX_PAYLOAD 2 
#define ID_SUPPLIER "id_supplier"

/**
 * @brief Processes requests from the Supplier component to retrieve product IDs with the 
 *        same minimum quantity for a specific supplier.
 * 
 * This function handles the extraction of the supplier ID from the Redis message payload 
 * and retrieves the IDs of products with the current minimum quantity for the specified 
 * supplier. It sends a response back to the `reply_stream` indicating the result of the 
 * operation.
 * 
 * @param c2r A pointer to the Redis context used for communication with the Redis server.
 * @param reply_stream The Redis stream where the reply to the Supplier component is sent.
 * @param reply A pointer to the Redis reply object containing the incoming request.
 * @param db An instance of the database connection object (`Con2DB`) used to query product information.
 * @param stream_index The index of the stream in the incoming Redis message.
 * @param message_index The index of the message within the stream to be processed.
 * @param num_val_msg The total number of key-value pairs in the message payload.
 * @param error A reference to the `HandleError` object for managing errors encountered during execution.
 *
 *  **Query the Database:**
 *    - Invokes the `infoCurrentProductQuantitiesQuery` function to retrieve the product IDs 
 *      with the same minimum quantity for the specified supplier.
 * 
 *  **Send a Response:**
 *    - If the query returns an empty result (indicating an error or no matching products):
 *        - Sends a `FAILED_REQ` message to the `reply_stream`.
 *    - If the query is successful:
 *        - Sends a `SUCCESS_REQ` message to the `reply_stream` containing the retrieved product IDs.
 */

void infoCurrentProductQuantities(redisContext *c2r, char* reply_stream, redisReply *reply,
                                  Con2DB db, size_t stream_index, size_t message_index,
                                  size_t num_val_msg, HandleError& error){

char key[100];
char value[100];
unsigned long id_supplier;
                                 
redisReply* replyCommand;
std::string msg;
std::vector<unsigned long> productIDs;

  //Extracting the supplier id.
 for(size_t h = INDEX_PAYLOAD; h < num_val_msg; h += 2){
                                             
          ReadStreamMsgVal(reply, stream_index, message_index, h, key);  
          ReadStreamMsgVal(reply, stream_index, message_index, h + 1, value);
          if(h == INDEX_PAYLOAD){   
                                                        
                 if(std::string(key) == ID_SUPPLIER && isPositiveInteger(value)){
                      id_supplier = std::stoul(value);  
                 }
                 else{
                      error.handleError(ERR_STREAM_PARSE_FAILED, "Incorrect id supplier format."); 
                 }                   
         }                                                
  }

  productIDs = infoCurrentProductQuantitiesQuery(db, id_supplier, msg);

  if(productIDs.empty()){
        
      std::cerr << "Product search error: " << msg << std::endl;
   
      std::stringstream command;
                        command << "XADD " << reply_stream
                                << " NOMKSTREAM * "
                                << "FAILED_REQ " << FAILED_REQ;

      replyCommand = (redisReply*)redisCommand(c2r, command.str().c_str());

      if(replyCommand == nullptr || replyCommand->type == REDIS_REPLY_ERROR){
         std::cerr << "Redis command error: " << c2r->errstr << std::endl;
         error.handleError(ERR_PROCESSING_FAILED, "Failed to send reply 'FAILED_REQ' to the client."); 
      } 
      
      freeReplyObject(replyCommand);
  }
  else{
      std::stringstream command;
                        command << "XADD " << reply_stream
                                << " NOMKSTREAM * "
                                << "SUCCESS_REQ " << SUCCESS_REQ;

      //Adds the IDs of the new suppliers to the reply
      for(const auto& id : productIDs){
            command << " id_product " << id;
      }
             
      replyCommand = (redisReply*)redisCommand(c2r, command.str().c_str());

      if(replyCommand == nullptr || replyCommand->type == REDIS_REPLY_ERROR){
          std::cerr << "Redis command error: " << c2r->errstr << std::endl;
          error.handleError(ERR_PROCESSING_FAILED, "Failed to send reply 'SUCCESS_REQ' to the client."); 
      }
    freeReplyObject(replyCommand);
  }
}




