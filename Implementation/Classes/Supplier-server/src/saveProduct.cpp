#include "main.h"

#define INDEX_PAYLOAD 2 
#define ID_SUPPLIER "id_supplier"
#define QUANTITY "quantity"

/**
 * @brief Handles the registration of a new product for a specific supplier in the database.
 *
 * This function is triggered when the Supplier component sends a request to register a new product.
 * It parses the incoming Redis message to extract the supplier ID and the initial quantity of the
 * product, then calls `saveProductQuery` to perform the database insertion. If successful, it sends
 * a reply containing the product ID back to the Supplier component. The operation is logged in the
 * log database for auditing purposes.
 *
 * Sends a Redis reply:
 *    - On success: Sends a `SUCCESS_REQ` message containing the `id_product`.
 *    - On failure: Sends a `FAILED_REQ` message.
 *
 * @param c2r            Pointer to the Redis context for executing Redis commands.
 * @param reply_stream   Name of the Redis stream where the server sends replies.
 * @param reply          Pointer to the Redis reply structure containing the message to parse.
 * @param db             Database connection object for managing product data.
 * @param logdb          Database connection object for logging operations.
 * @param stream_index   Index of the stream in the Redis reply array.
 * @param message_index  Index of the message in the stream to process.
 * @param num_val_msg    Number of elements in the Redis message payload.
 * @param error          Reference to a HandleError object for managing errors.
 *
 * The operation is logged in the log database using `log2db`.
 */

void saveProduct(redisContext *c2r, char* reply_stream, redisReply *reply,
                 Con2DB db, Con2DB logdb,size_t stream_index, size_t message_index,
                 size_t num_val_msg, HandleError& error){
                 
char key[100];
char value[100];                 
unsigned long id_supplier;
unsigned int product_quantity; 
    
redisReply* replyCommand;
std::string msg;
unsigned long id_product;
long nanosStart, nanosEnd; 

nanosStart = get_nanos();

//Extracting supplier id and quantity assigned to the new product to be registered
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
      else{ 
            if(std::string(key) == QUANTITY && isPositiveInteger(value)){
                 product_quantity = static_cast<unsigned int>(std::stoul(value));  
            }       
            else{
                  error.handleError(ERR_STREAM_PARSE_FAILED, "Wrong product id format or quantity.");
            }
      }
 }   

id_product =  saveProductQuery(db, id_supplier, product_quantity, msg);

//Checks whether the query in the database was successful
if(id_product == 0){
     std::cerr << "Error saving new product : " << msg << std::endl;
     std::stringstream command;
                       command <<"XADD "<<reply_stream
                               <<" NOMKSTREAM * "
                               <<"FAILED_REQ "<<FAILED_REQ;
     
     replyCommand = (redisReply*)redisCommand(c2r, command.str().c_str());

     if(replyCommand== nullptr || replyCommand->type == REDIS_REPLY_ERROR){
          std::cerr << "Redis command error: " << c2r->errstr << std::endl;
          error.handleError(ERR_PROCESSING_FAILED, "Attempt to send a reply 'FAILED_REQ' to the client failed."); 
     }
     freeReplyObject(replyCommand); 
 }
 else{
     std::stringstream command;
                        command <<"XADD "<<reply_stream
                                <<" NOMKSTREAM * "
                                <<"SUCCESS_REQ "<<SUCCESS_REQ
                                <<" id_product "<<id_product;
     
     replyCommand = (redisReply*)redisCommand(c2r, command.str().c_str());

     if(replyCommand == nullptr || replyCommand->type == REDIS_REPLY_ERROR){
         std::cerr << "Redis command error: " << c2r->errstr << std::endl;
         error.handleError(ERR_PROCESSING_FAILED, "Attempt to send a reply 'SUCCESS_REQ' and id product to the client failed."); 
     } 
     freeReplyObject(replyCommand);
    }

 nanosEnd = get_nanos();
 
 log2db(
    logdb,                          //db
    "save-product",                 //reqType
    std::make_optional<std::string>(std::to_string(id_supplier)), //usrID
    std::make_optional<std::string>(std::to_string(1)),           //usrState //user-state = generate_product
    std::nullopt,                   //reqID
    "SUPPLIER",                     //component
    "server::saveProduct",          //srcContext
    "INFO",                         //logLvl
    static_cast<unsigned>(getpid()), //pid
    std::make_optional<unsigned long>(nanosEnd - nanosStart), //durationNanos
   std::nullopt                    //logInfo
);

 
}

