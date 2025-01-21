#include "main.h"
#define INDEX_PAYLOAD 2 
#define ID_SUPPLIER "id_supplier"

/**
 * @brief Processes a request to update product quantities for an existing supplier.
 *
 * The function reads a Redis stream message sent by the Supplier component,
 * extracts the supplier ID and product details (IDs and quantities), and updates
 * the product quantities in the database. It utilizes `updateProductQuantitiesQuery`
 * to perform the update operation. After the update, the server sends a response
 * to the Supplier component.
 *
 * @param c2r Pointer to the Redis context for communication with Redis.
 * @param reply_stream Name of the Redis stream for sending replies to the client.
 * @param reply Pointer to the Redis reply containing the stream message.
 * @param db Database connection object for updating product quantities.
 * @param logdb Database connection object for logging the operation.
 * @param stream_index Index of the stream being processed.
 * @param message_index Index of the message being processed.
 * @param num_val_msg Total number of elements in the Redis message.
 * @param error Reference to the `HandleError` object for managing errors.
 *
 * Sends a Redis reply to indicate the outcome of the operation:
 *    - `SUCCESS_REQ` if the update was successful.
 *    - `FAILED_REQ` if the operation failed.
 * Logs the operation details (e.g., supplier ID, duration, status) into the log database.
 */

void updateProductQuantities(redisContext *c2r, char* reply_stream, redisReply *reply,
                             Con2DB db, Con2DB logdb, size_t stream_index, size_t message_index,
                             size_t num_val_msg, HandleError& error){

char key[100];
char value[100];
unsigned long id_supplier; 
std::unordered_map<unsigned long, unsigned int> products; 
redisReply* replyCommand;
std::string msg;
int res; 
long nanosStart, nanosEnd; //log

nanosStart = get_nanos();

  for(size_t h = INDEX_PAYLOAD ; h < num_val_msg; h += 2){
                                             
       ReadStreamMsgVal(reply, stream_index, message_index, h, key);  
       ReadStreamMsgVal(reply, stream_index, message_index, h + 1, value);
                                                  
       if(h == INDEX_PAYLOAD){  //Estrazione dell'id supplier 
                                                        
            if(std::string(key) == ID_SUPPLIER && isPositiveInteger(value)){
                  id_supplier = std::stoul(value);  
            }
            else{
                  error.handleError(ERR_STREAM_PARSE_FAILED, "Incorrect id supplier format."); 
            }                   
        }
        else{  
            if (isPositiveInteger(key) && isPositiveInteger(value)) {
                  products[std::stoul(key)] = static_cast<unsigned int>(std::stoul(value));  
            } 
            else{
                  error.handleError(ERR_STREAM_PARSE_FAILED, "Incorrect product id or quantity format.");
            }
        }
    }

res = updateProductQuantitiesQuery(db, id_supplier,products, msg);

if(res == EXIT_FAILURE){
        
      std::cerr << "Error updating product quantites " << msg << std::endl;
 
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

     replyCommand = (redisReply*)redisCommand(c2r, command.str().c_str());

     if(replyCommand == nullptr || replyCommand->type == REDIS_REPLY_ERROR){
           std::cerr << "Redis command error: " << c2r->errstr << std::endl;
           error.handleError(ERR_PROCESSING_FAILED, "Failed to send reply 'SUCCESS_REQ' to the client."); 
     }
    freeReplyObject(replyCommand);
    }


 nanosEnd = get_nanos();
 
log2db(
    logdb,                              //db
    "product-quantity-update",          //reqType
    std::make_optional<std::string>(std::to_string(id_supplier)), //usrID
    std::make_optional<std::string>(std::to_string(3)),           //usrState
    std::nullopt,                       //reqID
    "SUPPLIER",                         //component
    "server::updateProductQuantities",  //srcContext
    "INFO",                             //logLvl
    static_cast<unsigned>(getpid()),    //pid
    std::make_optional<unsigned long>(nanosEnd - nanosStart), //durationNanos
    std::nullopt                        //logInfo
);


}                        
                              
                              
