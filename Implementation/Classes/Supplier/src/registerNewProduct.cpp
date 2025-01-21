#include "main.h"
#define ID_PRODUCT "id_product"

namespace{
    char command[256]; 
    char stream_name[100];
    char key[100];
    char value[100];
    char msg_id[100];
    size_t num_msg;
    size_t num_val_msg;
    redisReply *reply;
}
/**
 * @brief Handles the registration of a new product associated with an existing supplier.
 *
 * This function formats a request for registering a new product, sends it to the 
 * server through a Redis stream, and waits for a response. The request includes:
 * - REQ TYPE::NEW_PRODUCT
 * - Supplier ID and initial quantity of the product
 *
 * The product ID is not provided by the client but is automatically generated 
 * during the registration process. The response is received on a specified reply 
 * stream and may include:
 * - SUCCESS_REQ: Contains the product ID of the newly registered product.
 * - FAILED_REQ: Indicates a server-side failure during registration.
 * - INVALID_FORMAT_REQ: Indicates that the request format was incorrect.
 *
 * @param c2r Redis context for executing commands.
 * @param consumer_grp The name of the consumer group processing the response.
 * @param block Maximum time to wait for a response in milliseconds.
 * @param req_stream Stream to send the registration request.
 * @param reply_stream Stream to receive the server's response.
 * @param error Error handling object for logging and managing errors.
 * @param id_supplier The ID of the supplier registering the product.
 * @param id_product Reference to store the generated product ID upon success.
 * @param quantities Initial quantity of the product.
 * @return STATUS_REQ: Indicates the result of the registration (e.g., SUCCESS_REQ, FAILED_REQ).
 */

STATUS_REQ registerNewProduct(redisContext *c2r,const char* consumer_grp, const int block, const char *req_stream, const char *reply_stream,
                              HandleError& error, const unsigned long id_supplier, unsigned long &id_product, const unsigned int quantities){
        
     snprintf(command, sizeof(command), "XADD %s NOMKSTREAM * NEW_PRODUCT %d id_supplier %lu quantity %u",
                                       req_stream, static_cast<int>(NEW_PRODUCT), id_supplier, quantities);
   
     reply = (redisReply *)redisCommand(c2r, command);
    
     if(reply == nullptr || reply->type == REDIS_REPLY_ERROR){
        std::cerr<< "Redis command error: "<< c2r->errstr<< std::endl;
        error.handleError(ERR_PROCESSING_FAILED, "Trying to send a request to the server for a new product");
     }

     freeReplyObject(reply);
     micro_sleep(500000); //0,5 s sleep
    
     reply =(redisReply *)redisCommand(c2r, "XREADGROUP GROUP %s Alice BLOCK %d COUNT 2 NOACK STREAMS %s >", consumer_grp, block, reply_stream);              
     
     //printf("main(): pid %d: user %s: Read msg from stream %s\n", getpid(), consumer_grp, reply_stream);
                 
     if(reply == nullptr){
        std::cerr<<"Error: NULL redisReply (error: "<< c2r->errstr << ")"<< std::endl;
        freeReplyObject(reply); 
        return NO_RESPONSE; // No response received; try again
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
                        for(size_t h = 2; h < num_val_msg; h += 2 ){
                      
                             ReadStreamMsgVal(reply, k, i, h, key);   
                             ReadStreamMsgVal(reply, k, i, h + 1, value);
                     
                             if(std::string(key) == ID_PRODUCT && isPositiveInteger(value)){
                                   id_product = std::stoul(value);  
                             }
                             else{
                                   freeReplyObject(reply);
                                   error.handleError(ERR_STREAM_PARSE_FAILED, "Incorrect id product format or code."); 
                             }      
                        }
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
           return NO_RESPONSE; // No response received; try again
      }      
    freeReplyObject(reply);
    return REQ_FAILED;  
}
