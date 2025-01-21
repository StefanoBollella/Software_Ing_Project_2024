#include "redis_utils.h"

/**
 * @brief Tests the connection to a Redis server by executing basic commands.
 *
 * This function verifies the connectivity to a Redis server by:
 * 1. Sending a `PING` command to check if the server is reachable.
 * 2. Setting and retrieving a key-value pair (`foo:bar`) to ensure basic operations.
 *
 * @param c2r A pointer to the Redis context representing the connection.
 * @return 0 if the connection is successful; -1 if there is an error.
 */
int connection_test(redisContext *c2r){
  
   redisReply *reply;
   
   reply = (redisReply *)redisCommand(c2r, "PING"); 
   if(!reply||c2r->err){
     // std::cerr << "Error: Can't send command to Redis." << std::endl;
      return -1;
   }

   freeReplyObject(reply);  
   
   reply = (redisReply *)redisCommand(c2r, "SET %s %s", "foo","bar"); 
   if(!reply||c2r->err){
     // std::cerr << "Error: Can't send command to Redis." << std::endl;
      return -1;
   }
    
   freeReplyObject(reply);  

   reply = (redisReply *)redisCommand(c2r, "GET foo"); 
   if(!reply||c2r->err || reply->type != REDIS_REPLY_STRING){
     // std::cerr << "Error: Can't send command to Redis." << std::endl;
      return -1;
   }
   freeReplyObject(reply);  

 return 0;
}
/**
 * @brief Initializes Redis streams for server or client usage.
 *
 * This function performs the following tasks:
 * 1. Tests the connection to Redis using `connection_test()`.
 * 2. Checks if the specified request and reply streams exist, and deletes them if they do.
 * 3. Initializes the streams with the provided group name.
 * 4. Verifies the successful initialization of both streams.
 *
 * @param c2r A pointer to the Redis context representing the connection.
 * @param error A reference to the `HandleError` instance for error handling.
 * @param groupName The name of the consumer group to associate with the streams.
 * @param req_stream The name of the request stream.
 * @param reply_stream The name of the reply stream.
 *
 * @throws std::runtime_error via `HandleError` if the connection test or stream initialization fails.
 */

void initialize(redisContext *c2r, HandleError &error,const char *groupName, char* req_stream, char* reply_stream){
    
    int conn_status = connection_test(c2r);
    if (conn_status != 0) { 
        error.handleError( ERR_CONNECTION_TEST_FAILED , "Error while checking connection to Redis.");
    }

    redisReply *reply;

     
    if(streamExists(c2r, req_stream)){
       
        reply = (redisReply *)redisCommand(c2r, "DEL %s", req_stream);
        assertReply(c2r, reply);  
        dumpReply(reply, 0);  
        freeReplyObject(reply);
    }
    else {
        printf("Stream %s does not exist, it will be created.\n", req_stream);
    }
    
    
    if (streamExists(c2r, reply_stream)) {
        
        reply = (redisReply *)redisCommand(c2r, "DEL %s", reply_stream);
        assertReply(c2r, reply);  
        dumpReply(reply, 0);  
        freeReplyObject(reply);
    } 
    else {
        printf("Stream %s does not exist, it will be created.\n", reply_stream);
    }

 
  initStreams(c2r, req_stream, groupName);  
   if (!streamExists(c2r, req_stream)) {
       error.handleError(ERR_STREAM_INIT_FAILED, "Error: Failed to initialize req_stream.");
    }
  
  
  initStreams(c2r, reply_stream, groupName);  
  if (!streamExists(c2r, reply_stream)) {
       error.handleError(ERR_STREAM_INIT_FAILED, "Error: Failed to initialize reply_stream.");
    }

   
    printf("Server ready to receive commands.\n");
}

//Controlla se una stringa rappresenta un intero positivo
 bool isPositiveInteger(const char* str){
    if(str == nullptr || strlen(str) == 0){
        return false; 
    }
    for(size_t i = 0; i < strlen(str); ++i){
        if (!isdigit(str[i])) {
            return false; 
        }
    }
 return true;
 }
