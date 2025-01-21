#include "main.h"

#define ID_SUPPLIER "id_supplier"
#define IND_STREAM 0
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
 * @brief Manages the registration of one or more new suppliers in the system.
 *
 * This function sends a request to register new suppliers using Redis streams. 
 * The function formats the request with the following details:
 * - REQ TYPE::SUPPLIER_ID
 * - The number of suppliers to register, specified by the `quantity` parameter.
 *
 * Supplier IDs are generated automatically upon successful registration in the database.
 * The function sends the request on the specified request stream (`req_stream`) and 
 * waits for a response on the reply stream (`reply_stream`). The responses may include:
 * - SUCCESS_REQ: Contains the generated IDs of the newly registered suppliers.
 * - FAILED_REQ: Indicates server-side failure during registration.
 * - INVALID_FORMAT_REQ: Suggests that the request did not meet the expected format.
 *
 * @param c2r Redis context for command execution.
 * @param consumer_grp The consumer group managing the response reading.
 * @param block The maximum waiting time (in milliseconds) for a response.
 * @param req_stream Stream to send the registration request.
 * @param reply_stream Stream to receive the server's response.
 * @param error Error handling object for logging and managing errors.
 * @param quantity Number of suppliers to register.
 * @param supplierIDs Vector to store the generated supplier IDs upon success.
 * @return STATUS_REQ: Returns the status of the registration (e.g., SUCCESS_REQ, FAILED_REQ).
 */

STATUS_REQ registerSupplier(redisContext *c2r, const char* consumer_grp, const int block , const char *req_stream, 
                            const char *reply_stream, HandleError& error, int quantity, std::vector<unsigned long> &supplierIDs){
           
     snprintf(command, sizeof(command), "XADD %s NOMKSTREAM * SUPPLIER_ID %d quantity %d", req_stream, static_cast<int>(SUPPLIER_ID), quantity);
     
     reply = (redisReply *)redisCommand(c2r, command);
     
     if(reply == nullptr || reply->type == REDIS_REPLY_ERROR){
        std::cerr<< "Redis command error: "<< c2r->errstr<< std::endl;
        error.handleError(ERR_PROCESSING_FAILED, "Trying to send request to get id for new supplier.");
     }
     
     freeReplyObject(reply);
     micro_sleep(500000); //0,5 s sleep
    
     //Reading on the reply_stream for the response to the command sent to the server
     reply =(redisReply *)redisCommand(c2r, "XREADGROUP GROUP %s Alice BLOCK %d COUNT 10 NOACK STREAMS %s >", consumer_grp, block, reply_stream);
     
     //printf("main(): pid %d: user %s: Read msg from stream %s\n", getpid(), consumer_grp, reply_stream);                         
     if(reply == nullptr) {
        std::cerr << "Error: NULL redisReply (error: " << c2r->errstr << ")" << std::endl;
        freeReplyObject(reply);  
        return NO_RESPONSE; // No response received; try again
     }
     //dumpReply(reply, 0);
                  
     if(reply->type == REDIS_REPLY_ARRAY && reply->elements > 0){
               
              ReadStreamName(reply, stream_name, IND_STREAM);   
              num_msg = ReadStreamNumMsg(reply,IND_STREAM);   
              
              if(num_msg > 0){
             
                 //Reads only the last message received from Supllier-server
                 size_t last_msg_index = num_msg - 1;
                 ReadStreamNumMsgID(reply, IND_STREAM, last_msg_index, msg_id);
                 
                 num_val_msg  = ReadStreamMsgNumVal(reply,IND_STREAM,last_msg_index); 
              
                 REPLY_TYPE replyType = parseReply(reply,IND_STREAM,last_msg_index,error);
                   
                 if(replyType == SUCCESS_REQ){
                  
                     for(size_t h = 2; h < num_val_msg; h += 2 ){
                           ReadStreamMsgVal(reply, IND_STREAM, last_msg_index, h, key);   
                           ReadStreamMsgVal(reply, IND_STREAM, last_msg_index, h + 1, value);
                     
                           if(std::string(key) == ID_SUPPLIER && isPositiveInteger(value)){
                                  supplierIDs.push_back(std::stoul(value));
                           }   
                           else{
                                   freeReplyObject(reply);
                                   error.handleError(ERR_STREAM_PARSE_FAILED, "Incorrect command code format.");
                           }
                      }
                      freeReplyObject(reply);
                      return REQ_SUCCESS; 
                 }
                 else if(replyType == FAILED_REQ){
                        freeReplyObject(reply);
                        return  REQ_FAILED;  
                 }
                 else{ //INVALID_FORMAT_REQ : Request with wrong format, parsing not possible 
                      freeReplyObject(reply);
                      return  BAD_REQUEST;       
                 }   
            }
            else{                 
               freeReplyObject(reply);
               return NO_RESPONSE; // No response received; try again
           }
    } //if
    else {    
        //No elements found in the reply, likely no response
        freeReplyObject(reply);
        return NO_RESPONSE; // No response received; try again
    }      
  freeReplyObject(reply);  
  return REQ_FAILED;
}
