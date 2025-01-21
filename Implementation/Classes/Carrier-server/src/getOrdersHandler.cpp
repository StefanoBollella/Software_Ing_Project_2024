#include "main.h"

#define BUFF_SIZE 100

void getOrdersHandler(redisContext* c2r, redisReply* readReply, char* replyStream, Con2DB db, int ithMsg, Con2DB logdb){

    char fval[BUFF_SIZE]; 
    std::string msg;
    redisReply* sendReply;
    unsigned long ordsNum;
    std::vector<unsigned long> ordersID;
    int res;
    unsigned long carrID;
    long nanosStart, nanosEnd;

    nanosStart = get_nanos();

    /*
     * request: type - id - quantity -
     * reply: type - id - numords - code - msg - ordids - 
    */
    // reads the value of the number of orders requested
    ReadStreamMsgVal( readReply, STREAM_IND, ithMsg, req_formats::GORDREQ_ORD_QTY_V, fval);
    ordsNum = std::stoul(fval);
    memset(fval, '\0', sizeof(fval));

    // gets the carrierID
    ReadStreamMsgVal( readReply, STREAM_IND, ithMsg, req_formats::GORDREP_CARR_ID_V, fval);
    carrID = std::stoul(fval);
    memset(fval, '\0', sizeof(fval));  

    // number of orders request is zero
    if (ordsNum == 0) {
        sendReply = RedisCommand(c2r,
            "XADD %s NOMKSTREAM * type %u carr_id %lu "
            "n_ords %u code %u msg %s ordsRcvd []",
            replyStream, reqstatus::GET_ORDS_REQ, carrID,
            ordsNum, reqstatus::BAD_REQ, "n_ords > 0 not satisfied"
        );
    }  

    // query
    ordersID = getOrdersQuery(db, ordsNum, msg, &res, carrID); 
    std::cout << "ordersID: " << vecToStr(ordersID).c_str() << std::endl;
    // query success 
    if (res == EXIT_SUCCESS) {
        sendReply = RedisCommand(c2r,
            "XADD %s NOMKSTREAM * type %u carr_id %lu "
            "n_ords %u code %u msg %s ordsRcvd %s",
            replyStream, reqstatus::GET_ORDS_REQ, carrID,
            ordsNum, reqstatus::REQ_OK, "get orders success",
            vecToStr(ordersID).c_str()
        );

    } else {
        sendReply = RedisCommand(c2r,
            "XADD %s NOMKSTREAM * type %u carr_id %lu "
            "n_ords %u code %u msg %s ordsRcvd []",
            replyStream, reqstatus::GET_ORDS_REQ, carrID,
            ordsNum, reqstatus::BAD_REQ, msg.c_str() 
        );
    }

    assertReplyType(c2r, sendReply, REDIS_REPLY_STRING);
    dumpReply(sendReply, 1);
    freeReplyObject(sendReply);
    
    nanosEnd = get_nanos();

    if(res == EXIT_SUCCESS){
        log2db(
            logdb, "get-order", 
            std::make_optional<std::string>(std::to_string(carrID)),
            std::make_optional<std::string>(std::to_string(1)), // usr_state = ready
            std::nullopt,
            "CARRIER", "server::getOrdersHandler", "INFO", getpid(),
            std::make_optional<unsigned long>(nanosEnd - nanosStart),
            std::nullopt
        );
    }

}