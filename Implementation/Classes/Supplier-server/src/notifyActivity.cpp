#include "main.h"

#define REQ_PONG_STREAM_IND 0  //Stream position index req_pong_supplier

/**
 * @brief Handles communication between the supplier server and the monitor
 * to ensure active monitoring of the server's state via PING-PONG messages.
 *
 * It reads PING requests from the `req_pong_supplier` Redis stream, verifies the message's content,
 * and sends back a PONG response to the `reply_ping` stream if the request is valid.
 *
 * @param c2r              Pointer to the Redis context for executing commands.
 * @param req_pong_supplier Name of the stream from which the supplier server reads PING requests.
 * @param reply_ping        Name of the stream to which the server sends PONG responses.
 */

void notifyActivity(redisContext *c2r, const char* req_pong_supplier, const char* reply_ping){
   
   const char* SERVER_ID = "server_supplier"; 
   const char* MONITOR_ID = "monitor"; 
   const char CONSUMER_GROUP_MONITOR[] = "monitor-grp-0";
   redisReply *reply;
   
   if(streamExists(c2r, reply_ping) && streamExists(c2r, req_pong_supplier)){
        
         //Reads the ping request from monitor
         //Modifica : COUNT passa da 1  a 2 
        reply = (redisReply*)redisCommand(c2r, "XREADGROUP GROUP %s supplier_server COUNT 2 NOACK STREAMS %s >", CONSUMER_GROUP_MONITOR, req_pong_supplier);
        
        if (reply == nullptr) {
            std::cerr << "Error: Unable to read stream." << std::endl;
            return;
        }
        
         //Checks that there are messages in the stream req_pong_supplier
        if(reply->type == REDIS_REPLY_ARRAY && reply->elements > 0){
            //dumpReply(reply, 0);

            size_t num_msg = ReadStreamNumMsg(reply,REQ_PONG_STREAM_IND);
               
                long int num_seq_ping = 0; 
                char monitor_id[100];
                char value[100];
                
                ReadStreamMsgVal(reply, REQ_PONG_STREAM_IND, num_msg -1, 0, monitor_id);//Monitor_id extraction
                ReadStreamMsgVal(reply, REQ_PONG_STREAM_IND, num_msg -1, 1, value); //Extraction num_seq_ping

                if(std::string(monitor_id) == MONITOR_ID && isPositiveInteger(value)){
                    num_seq_ping = std::stol(value);
                    
                   //Sending the PONG to the monitor
                    char command[256];
                    snprintf(command, sizeof(command), "XADD %s NOMKSTREAM * %s %ld", reply_ping, SERVER_ID, num_seq_ping);
                    redisReply* pong_reply = (redisReply*)redisCommand(c2r, command);
                    
                    if (pong_reply == nullptr) {
                        std::cerr << "Error: Unable to send PONG to monitor." << std::endl;
                    }
                    
                    freeReplyObject(pong_reply);
                    num_seq_ping  = 0; 
                }
            //}
        }                
       freeReplyObject(reply);
    }
}
