#include "main.h"
#define MIN_ARG 6  //exec_file, req_stream, reply_stream, blocking_time, reply_ping, req_pong_supplier 
#define STREAM_MAX_LEN 50
#define MAX_BLOCKING 6000 //milleseconds
#define REQ_STREAM_IND 0  //Stream position index req_stream

using namespace std;

namespace{

   redisContext *c2r; 
   redisReply *reply; 
   char *req_stream; 
   char *reply_stream;
   char *reply_ping;
   char *req_pong_supplier;
   
   int block; 
   int pid; 
   char stream_name[100];
   char msg_id[100];
   size_t num_msg;
   size_t num_val_msg;
  
   const char HOST_NAME[] = "localhost";
   const unsigned int PORT_NUM = 6379;  
   const char CONSUMER_GROUP_SUPPLIER[] = "supplier-grp-0";
   
   Con2DB db("localhost", "5432", "ecommerce_db_usr", "47002", "ecommerce_db");
   Con2DB logdb("localhost", "5432", "logdb_usr", "47002", "logdb");

}//namespace

/**
 * Handles the SIGINT signal for graceful termination. 
 * Cleans up active Redis connections (`c2r`) and attempts to delete streams (`req_stream`, `reply_stream`) 
 * with retries, ensuring all resources are freed before exiting the program successfully.
 */
void handleSignal(int signal){
    if (signal == SIGINT){
        std::cout<<"Received SIGINT, cleaning up redis streams..."<< std::endl;

        if(c2r != nullptr){ 
        
               redisReply* delReply; 
                int attempts = 0;
                bool reply_stream_deleted = false;
                bool req_stream_deleted = false; 
                
                while(attempts < 10 && (!reply_stream_deleted || !req_stream_deleted)){ //Ripete per ora fino a 3 volte 
                    
                      if(!reply_stream_deleted){
                           delReply = (redisReply*)redisCommand(c2r, "DEL %s", reply_stream );
                               
                          if(delReply != nullptr && !streamExists(c2r, reply_stream) ){
                          
                             reply_stream_deleted = true;
                             std::cout<<"reply_stream deleted successfully."<< std::endl;
                          }
                       freeReplyObject(delReply);
                      }
                      if(!req_stream_deleted){
                           delReply = (redisReply*)redisCommand(c2r, "DEL %s", req_stream );
                               
                          if(delReply != nullptr && !streamExists(c2r, req_stream) ){
                          
                             req_stream_deleted = true;
                             std::cout<<"req_stream deleted successfully."<< std::endl;
                          }
                       freeReplyObject(delReply);
                      }          
                 attempts++;
              }
            redisFree(c2r);    
        }
        exit(EXIT_SUCCESS);
    }
}


int main(int argc, char **argv) {

    if(argc < MIN_ARG){ 
        std::cerr << "main(): usage: exec_file, req_stream, reply_stream, blocking_time, reply_ping, req_pong_supplier " << std::endl;
        exit(EXIT_FAILURE);
    }
    
    //Registra il gestore del segnale SIGINT
    std::signal(SIGINT, handleSignal);
    
    if(strlen(argv[1]) > STREAM_MAX_LEN){
           std::cerr << "Error: req_stream exceeds the maximum length of " 
                << STREAM_MAX_LEN << " characters." << std::endl;
            return EXIT_FAILURE;
    }
    req_stream = argv[1];

    if(strlen(argv[2]) > STREAM_MAX_LEN){
           std::cerr << "Error: reply_stream exceeds the maximum length of " 
                << STREAM_MAX_LEN << " characters." << std::endl;
            return EXIT_FAILURE;
    }
    reply_stream = argv[2];

    block = atoi(argv[3]);
    if(block > MAX_BLOCKING){
            std::cerr << "Error: block exceeds " << MAX_BLOCKING << std::endl;
            return EXIT_FAILURE;
    }
    
    if(strlen(argv[4]) > STREAM_MAX_LEN){
           std::cerr << "Error:  reply_ping exceeds the maximum length of " 
                << STREAM_MAX_LEN << " characters." << std::endl;
            return EXIT_FAILURE;
    }
     reply_ping = argv[4];
   
   if(strlen(argv[5]) > STREAM_MAX_LEN){
           std::cerr << "Error: req_pong_supplier exceeds the maximum length of " 
                << STREAM_MAX_LEN << " characters." << std::endl;
            return EXIT_FAILURE;
    }
    req_pong_supplier = argv[5];
    
    
   pid = getpid();     
   HandleError error;   
   ServerState server; //The status of the server is INITIALIZING 
  
   try{ 
        std::cout << "main(): pid "<< pid << ": connecting to redis..." <<std::endl;  
        c2r = redisConnect(HOST_NAME, PORT_NUM);
   
        if(c2r == nullptr || c2r->err){
             error.handleError(ERR_CONNECTION_FAILED, "Redis connection error."); 
        }   
        std::cout << "main(): pid "<< pid << ": connected to redis." <<std::endl; 
          
        server.updateServerState(ServerStatus::CONNECTED);  
        
        //Server initialization 
        initialize(c2r, error, CONSUMER_GROUP_SUPPLIER, req_stream, reply_stream);  
       
        server.updateServerState(ServerStatus::READY);    
       
        std::cout << "main(): pid "<< pid << ": Server is ready." <<std::endl; 
        
              
        char buf[200];
        int iteration = 0; 
        init_time();
        nanos = get_nanos();
        
        printf("%-10s %-15s %-15s %-15s\t %-20s %-15s%-15s\n", 
               "Iteration", "Global Time", "Timeadvance", "Nanos", "Buffer", "Nanos Day", "Current Req");
  
        while(server.getCurrentState() == ServerStatus::READY){            
              
                try{              
                          nanos_day = get_day_nanos(buf); 
                         
                          printf("%-10d %-15.5lf %-15.5lf %-20ld %-20s %-15ld ", 
                                              iteration, global_time_sec, timeadvance, nanos, buf, nanos_day);
                         
                          notifyActivity(c2r, req_pong_supplier, reply_ping);
                                         
                          server.updateServerState(ServerStatus::BUSY); 
                                           
                          reply =(redisReply *)redisCommand(c2r, "XREADGROUP GROUP %s server BLOCK %d COUNT 2 NOACK STREAMS %s >", CONSUMER_GROUP_SUPPLIER, block, req_stream);
                           
                          //printf("main(): pid %d: user %s: Read msg %d from stream %s\n", pid, CONSUMER_GROUP, read_counter, req_stream);
                         
                          if(reply == nullptr){
                            std::cerr << "Error: NULL redisReply (error: " << c2r->errstr << ")" << std::endl;
                            freeReplyObject(reply);  
                            error.handleError(ERR_PROCESSING_FAILED, "Error during request reading.");
                          }
                          
                          //dumpReply(reply, 0);
                          
                          if(reply->type == REDIS_REPLY_ARRAY && reply->elements > 0){
                       
                                  ReadStreamName(reply, stream_name, REQ_STREAM_IND); 
                                  num_msg = ReadStreamNumMsg(reply,REQ_STREAM_IND); 
                      
                                  for(size_t i = 0; i < num_msg; i++ ){            
                                    
                                      ReadStreamNumMsgID(reply, REQ_STREAM_IND,i, msg_id);
                                      num_val_msg  = ReadStreamMsgNumVal(reply, REQ_STREAM_IND, i);  
                    
                                     //Extracting client request type
                                      REQ_TYPE reqType = parseRequest(reply,REQ_STREAM_IND,i,error);
                                      
                                      printf("%-15s\n", HandleRequest::reqTypeToString(reqType).c_str());
                         
                                      if (reqType == REQ_TYPE::SUPPLIER_ID) {
                                                //Registration of new suppliers
                                                registerSuppliers(c2r, reply_stream, reply, db, REQ_STREAM_IND, i, num_val_msg, error);
                                      } 
                                      else if (reqType == REQ_TYPE::NEW_PRODUCT) {
                                                //Registration of the new product
                                                saveProduct(c2r, reply_stream, reply, db, logdb, REQ_STREAM_IND, i, num_val_msg, error);
                                      } 
                                      else if (reqType == REQ_TYPE::INFO_PRODUCT) {
                                                //Information on quantities of products currently registered
                                               infoCurrentProductQuantities(c2r, reply_stream, reply, db, REQ_STREAM_IND, i, num_val_msg, error);
                                      } 
                                      else if (reqType == REQ_TYPE::UPDATE_PRODUCT) {
                                               //Updating product quantities
                                               updateProductQuantities(c2r, reply_stream, reply, db, logdb, REQ_STREAM_IND, i, num_val_msg, error);
                                      } 
                                   }//for2 
                          } //if
                          else {    
                                 error.handleError(ERR_STREAM_PARSE_FAILED, "Unable to parse stream data.");
                          }     
                      }
                      catch(const std::runtime_error& e){
                            printf("%-15s ",e.what());
                            ErrorCode code = error.getLastErrorCode();
                           
                            if(code == ErrorCode::ERR_PROCESSING_FAILED){ 
		                printf("During the process of reading the request stream.\n"); 
                            } 
                            else if(code == ErrorCode::ERR_STREAM_PARSE_FAILED){ 
		                     
		                     if(reply->type == REDIS_REPLY_NIL){
                                         printf("No messages available in stream.\n"); 
                                     } 
                                     else{
                                            //Errors during the parse phase of messages from the request stream.
                                            std::stringstream command;
                                                              command <<"XADD "<<reply_stream
                                                                      <<" NOMKSTREAM * "
                                                                      <<"INVALID_FORMAT_REQ "<<INVALID_FORMAT_REQ;
                                                                      
                                            reply = (redisReply*)redisCommand(c2r, command.str().c_str());

                                            if(reply == nullptr || reply->type == REDIS_REPLY_ERROR){
                                                 printf("Attempt to send a reply 'INVALID_FORMAT_REQ'.\n");
                                            }
                                          
                                    }
                             } 
                       } 
           server.updateServerState(ServerStatus::READY); 
           freeReplyObject(reply);
           micro_sleep(500000); //sleep 0.5 secondi   
           ++iteration;
           update_time();
      }//while 
    } 
    catch(const std::runtime_error& e){
           /* Critical errors due to connection to redis or failed initialization. 
            * No recovery is implemented.
            */
           std::cerr << "Critical error during server startup: " << e.what() << std::endl;
           server.updateServerState(ServerStatus::TERMINATED);
           if(c2r != nullptr){
              redisFree(c2r);  
           }
           std::cout << "Server terminated because of critical error." << std::endl;
           return EXIT_FAILURE; 
    }
     
   if(server.getCurrentState() != ServerStatus::TERMINATED){
      server.updateServerState(ServerStatus::TERMINATED);
   }
   redisFree(c2r);
   std::cout << "Server terminated successfully."<< std::endl;
return EXIT_SUCCESS;
}
