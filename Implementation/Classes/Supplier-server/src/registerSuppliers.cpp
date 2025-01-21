#include "main.h"
#define INDEX_PAYLOAD 2 
#define QUANTITY "quantity"

/**
 * @brief Registers a specified number of new suppliers in the database and replies to the client.
 *
 * This function processes a request to register a given quantity of new suppliers.
 * It extracts the requested quantity from the Redis stream message, invokes 
 * `registerSuppliersQuery` to add the suppliers to the database, and sends a response
 * back to the client with the result of the operation.
 *
 * @param c2r              Redis context for communication with the Redis server.
 * @param reply_stream     Redis stream for sending the reply back to the client.
 * @param reply            Redis reply object containing the incoming message.
 * @param db               Database connection for executing supplier registration queries.
 * @param stream_index     Index of the stream within the Redis reply.
 * @param message_index    Index of the message within the stream.
 * @param num_val_msg      Total number of key-value pairs in the message payload.
 * @param error            Reference to a `HandleError` instance for managing errors.
 *
 * Response:
 * - `SUCCESS_REQ`: Contains the IDs of the newly registered suppliers.
 * - `FAILED_REQ`: Indicates the registration failed due to an error.
 */

void registerSuppliers(redisContext *c2r, char* reply_stream, redisReply *reply,
                       Con2DB db, size_t stream_index, size_t message_index,
                       size_t num_val_msg, HandleError& error){

char key[100];
char value[100];
int supplier_quantity; 

std::vector<unsigned long> supplierIDs;
redisReply* replyCommand;
std::string msg;

 //Extracting number of suppliers to be registered
 for(size_t h = INDEX_PAYLOAD; h < num_val_msg; h += 2){
                                             
     ReadStreamMsgVal(reply, stream_index, message_index, h, key);    
     ReadStreamMsgVal(reply, stream_index, message_index, h + 1, value);  
                                             
     if(std::string(key) == QUANTITY && isPositiveInteger(value)){
               supplier_quantity = std::stoi(value); 
     } 
     else{
          error.handleError(ERR_STREAM_PARSE_FAILED, "Incorrect quantity format.");
     }
  }

 supplierIDs = registerSuppliersQuery(db, supplier_quantity, msg);

 if(supplierIDs.empty()){
        
      std::cerr << "Error registering suppliers: " << msg << std::endl;
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
        for(const auto& id : supplierIDs){
            command << " id_supplier " << id;
        }
        
        replyCommand = (redisReply*)redisCommand(c2r, command.str().c_str());

        if(replyCommand == nullptr || replyCommand->type == REDIS_REPLY_ERROR){
            std::cerr << "Redis command error: " << c2r->errstr << std::endl;
            error.handleError(ERR_PROCESSING_FAILED, "Failed to send reply 'SUCCESS_REQ' to the client."); 
        }
      freeReplyObject(replyCommand);
 }
}

