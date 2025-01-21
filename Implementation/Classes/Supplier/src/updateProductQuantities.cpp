#include "main.h"
namespace{
     char command[5000];                    
     char stream_name[100];
     char msg_id[100];
     size_t num_msg;
     size_t num_val_msg;
     redisReply *reply;
}
 /**
 * @brief Updates the quantities of products registered in the system.
 * 
 * This function manages the update of product quantities for a specific supplier.
 * It constructs a Redis command using the provided supplier ID and a map of product IDs
 * to quantities. The command is sent to the Redis server via the 
 * `req_stream`. After sending the command, the function waits for a response on the 
 * `reply_stream` to determine the result of the operation.
 * 
 * @param c2r A pointer to the Redis context used for communication with the Redis server.
 * @param consumer_grp The name of the consumer group used for reading responses.
 * @param block The maximum time (in milliseconds) to block when reading from the stream.
 * @param req_stream The Redis stream where the request is sent.
 * @param reply_stream The Redis stream where the response is expected.
 * @param error Reference to a HandleError object for managing errors encountered during execution.
 * @param id_supplier The ID of the supplier whose products are being updated.
 * @param productQuantities An unordered map containing product IDs (keys) and quantities 
 *                          to add (values).
 * @return STATUS_REQ Indicates the status of the operation:
 * - `REQ_SUCCESS` if the product quantities were successfully updated in the database.
 * - `REQ_FAILED` if the update failed due to a server-side error.
 * - `BAD_REQUEST` if the request format was invalid.
 * - `NO_RESPONSE` if no response was received from the server.
 */
STATUS_REQ  updateProductQuantities(redisContext *c2r, const char* consumer_grp, const int block , const char *req_stream, 
                                   const char *reply_stream, HandleError& error, const unsigned long id_supplier,
                                   std::unordered_map<unsigned long, unsigned int> &productQuantities){
                                   
      std::stringstream payload;
      for (const auto& pair : productQuantities) {
        payload << " " << pair.first << " " << pair.second;
      }                            
                                   
      int dim = snprintf(command, sizeof(command), "XADD %s NOMKSTREAM * UPDATE_PRODUCT %d id_supplier %lu %s", 
                              req_stream, static_cast<int>(UPDATE_PRODUCT), id_supplier,payload.str().c_str());
           
     //Checks whether snprintf was successful or exceeded the buffer size
     if(dim < 0 || dim >= static_cast<int>(sizeof(command))){
        std::cerr << "Error: Command exceeds buffer size or snprintf failed." << std::endl;
        error.handleError(ERR_PROCESSING_FAILED, "Constructing the Redis command.");
        return REQ_FAILED;
     }
          
     reply = (redisReply *)redisCommand(c2r, command);
     
     if(reply == nullptr || reply->type == REDIS_REPLY_ERROR){
            std::cerr<< "Redis command error: "<< c2r->errstr<< std::endl;
            error.handleError(ERR_PROCESSING_FAILED, "Trying to send request to update product quantities.");
     }
    
     freeReplyObject(reply);
     micro_sleep(500000); //0,5 s sleep
    
     //Reading on the reply_stream for the result of the command sent to the server
     reply =(redisReply *)redisCommand(c2r, "XREADGROUP GROUP %s Alice BLOCK %d COUNT 2 NOACK STREAMS %s >", consumer_grp, block, reply_stream);              
     
     //printf("main(): pid %d: user %s: Read msg from stream %s\n", getpid(), consumer_grp, reply_stream);
                 
     if(reply == nullptr){
        std::cerr<<"Error: NULL redisReply (error: "<< c2r->errstr << ")"<< std::endl;
        freeReplyObject(reply); 
        return NO_RESPONSE; // No response received, try again
     }
     //dumpReply(reply, 0);
                  
     if(reply->type == REDIS_REPLY_ARRAY && reply->elements > 0){
               
          for(size_t k = 0; k < reply->elements; k++){ 
                    
                 ReadStreamName(reply, stream_name, k); 
                 num_msg = ReadStreamNumMsg(reply,k);    
 
                 for(size_t i = 0; i < num_msg; i++ ){           
                            
                       ReadStreamNumMsgID(reply, k,i, msg_id);
                       num_val_msg  = ReadStreamMsgNumVal(reply, k, i);  
                       REPLY_TYPE replyType = parseReply(reply,k,i,error);
                     
                        if(replyType == SUCCESS_REQ){
                             freeReplyObject(reply);
                             return REQ_SUCCESS;  
                        }
                        else if(replyType == FAILED_REQ){
                           freeReplyObject(reply);
                           return  REQ_FAILED;  
                       }
                       else{ 
                          freeReplyObject(reply);
                          return  BAD_REQUEST;       
                       }                                         
                 }//for2
          }//for1
      } //if
      else {    
            //No elements found in the reply, likely no response
            freeReplyObject(reply);
           return NO_RESPONSE; //No response received; try again
      }      
    freeReplyObject(reply);
    return REQ_FAILED;  
}
                                
                                   
                                   
                                              
