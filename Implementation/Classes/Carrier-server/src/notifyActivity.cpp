#include "main.h"

void notifyActivity(redisContext *c2r, const char* ping_carrier, const char* pong_carrier){
   
   const char* SERVER_ID = "server_carrier"; 
   const char* MONITOR_ID = "monitor"; 
   const char CONSUMER_GROUP_MONITOR[] = "monitor-grp-0";
   // const int STREAM_IND = 0;  //indice posizione stream read_ping 
   redisReply *reply;
   
   if(streamExists(c2r, pong_carrier) && streamExists(c2r, ping_carrier)){
        
        // Lettura PING da parte del monitor
        // COUNT = 2 perchè cosi se il server ha dei PING arretrati ne legge 2 alla volta e raggiunge il monitor, senno se ne legge sempre solo 1
        // finisce che il server se arriva una volta in ritardo rimane sempre in ritardo
        reply = (redisReply*)redisCommand(c2r, "XREADGROUP GROUP %s carrier_server COUNT 2 NOACK STREAMS %s >", CONSUMER_GROUP_MONITOR, ping_carrier);
        
        if (reply == nullptr) {
            std::cerr << "Errore: impossibile leggere lo stream." << std::endl;
            return;
        }
        
        // Verifica che ci siano messaggi nello stream ping_carrier
        if(reply->type == REDIS_REPLY_ARRAY && reply->elements > 0){
            
            // dumpReply(reply, 0);

            size_t num_msg = ReadStreamNumMsg(reply, STREAM_IND);         
            long int num_seq_ping = 0; // valore corrente di num_seq_ping
            char monitor_id[100];
            char value[100];
                
            ReadStreamMsgVal(reply, STREAM_IND, num_msg -1, 0, monitor_id); // Estrazione monitor_id
            ReadStreamMsgVal(reply, STREAM_IND, num_msg -1, 1, value); // Estrazione num_seq_ping

            if(std::string(monitor_id) == MONITOR_ID && isPositiveInteger(value)){
                num_seq_ping = std::stol(value);
                    
                // Invio del PONG al monitor
                char command[256];
                snprintf(command, sizeof(command), "XADD %s NOMKSTREAM * %s %ld", pong_carrier, SERVER_ID, num_seq_ping);
                redisReply* pong_reply = (redisReply*)redisCommand(c2r, command);
                    
                if (pong_reply == nullptr) {
                    std::cerr << "Errore: impossibile inviare il PONG al monitor." << std::endl;
                }
                    
                freeReplyObject(pong_reply);
            }
        }
         
        
        freeReplyObject(reply);
    }
}

