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
 * @brief Retrieves the IDs of products with the minimum quantity recorded for a specific supplier.
 *
 * This function sends a Redis request to the server to obtain the IDs of products with the 
 * lowest quantity currently registered in the database for a specific `id_supplier`.
 * The request is sent via the `req_stream`, and the response is awaited on the `reply_stream`.
 * 
 * @param c2r               Redis context for communication with the server.
 * @param consumer_grp      Name of the consumer group to read messages from the reply stream.
 * @param block             Timeout (in milliseconds) for the XREADGROUP command on the reply stream.
 * @param req_stream        Name of the stream used to send the request to the server.
 * @param reply_stream      Name of the stream used to read the response from the server.
 * @param error             Reference to a HandleError instance to manage potential errors.
 * @param id_supplier       Supplier ID for which the products with the minimum quantity are requested.
 * @param ProductIDs        Output vector to be populated with the retrieved product IDs.
 * 
 * @return STATUS_REQ       Status of the request. Possible values:
 *                          - REQ_SUCCESS: The request was successfully completed, and 
 *                            `ProductIDs` was populated.
 *                          - REQ_FAILED: The request failed due to a server-side error.
 *                          - BAD_REQUEST: The request format was invalid.
 *                          - NO_RESPONSE: No response was received from the server.
 */
STATUS_REQ  findProductsQuatity(redisContext *c2r, const char* consumer_grp, const int block , const char *req_stream, 
                        const char *reply_stream, HandleError& error, const unsigned long id_supplier, std::vector<unsigned long> &ProductIDs){
                  
        snprintf(command, sizeof(command), "XADD %s NOMKSTREAM * INFO_PRODUCT %d id_supplier %lu", req_stream, static_cast<int>(INFO_PRODUCT), id_supplier);
            
        reply = (redisReply *)redisCommand(c2r, command);
     
        if(reply == nullptr || reply->type == REDIS_REPLY_ERROR){
            std::cerr<< "Redis command error: "<< c2r->errstr<< std::endl;
            error.handleError(ERR_PROCESSING_FAILED, "Sending a search request for product IDs with the lowest quantity for the indicated supplier.");
        }
        
        freeReplyObject(reply);
        micro_sleep(500000); //0,5 s sleep
    
       //Reading on the reply_stream for the result of the command sent to the server
        reply =(redisReply *)redisCommand(c2r, "XREADGROUP GROUP %s Alice BLOCK %d COUNT 2 NOACK STREAMS %s >", consumer_grp, block, reply_stream);
                   
        //printf("main(): pid %d: user %s: Read msg from stream %s\n", getpid(), consumer_grp, reply_stream);
                 
        if(reply == nullptr){
           std::cerr<<"Error: NULL redisReply (error: "<< c2r->errstr << ")"<< std::endl;
           freeReplyObject(reply);  
           return NO_RESPONSE; // No response received; try again
        }
        //dumpReply(reply, 0);
                  
        if(reply->type == REDIS_REPLY_ARRAY && reply->elements > 0){
               
         for(size_t k = 0; k < reply->elements; k++){ //Naviga il vettore degli stream 
                    
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
                            
                             if (std::string(key) == ID_PRODUCT && isPositiveInteger(value)) {
                                   ProductIDs.push_back(std::stoul(value));
                             } 
                             else {
                                    freeReplyObject(reply);
                                    error.handleError(ERR_STREAM_PARSE_FAILED, "Incorrect product ID format or code.");
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
         
        
