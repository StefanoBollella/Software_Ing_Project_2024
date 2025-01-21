#include "main.h"

#define BUFF_SIZE 500

void loseOrdHandler(redisContext* c2r, redisReply* readReply, char* replyStream, Con2DB db, int ithMsg, Con2DB logdb){

    char fval[BUFF_SIZE];
    redisReply* sendReply;
    unsigned long orderID, carrierID;
    std::string msg;
    int resQuery;
    long nanosStart, nanosEnd;

    nanosStart = get_nanos();

    // carrierID
    ReadStreamMsgVal(
        readReply, STREAM_IND, ithMsg, req_formats::LREQ_CARR_ID_V , fval
    );
    carrierID = std::stoul(fval, nullptr);
    // carrierID = fval;
    memset(fval, '\0', sizeof(fval));

    //orderID
    ReadStreamMsgVal(
        readReply, STREAM_IND, ithMsg, req_formats::LREQ_ORD_ID_V , fval
    );
    orderID = std::stoul(fval, nullptr);
    // orderID = fval;
    memset(fval, '\0', sizeof(fval));

    //call the function the exec the query
    resQuery = loseOrdQuery(
        db, carrierID, orderID, msg
    );

    if(resQuery == EXIT_SUCCESS) {  // query returns with success 

        sendReply = RedisCommand(
            c2r, "XADD %s NOMKSTREAM * type %u carrierID %lu orderID %lu code %u msg %s", 
            replyStream, reqstatus::LOSE_ORD_REQ, carrierID, orderID, reqstatus::REQ_OK, "order successfully lost" 
        );

    }   else {                      // query returns failure 

        sendReply = RedisCommand(
            c2r, "XADD %s NOMKSTREAM * type %u carrierID %lu orderID %lu code %u msg %s", 
            replyStream, reqstatus::LOSE_ORD_REQ, carrierID, orderID, reqstatus::BAD_REQ, msg.c_str() 
        );

    }

    assertReply(c2r, sendReply);
    assertReplyType(c2r, sendReply, REDIS_REPLY_STRING);
    freeReplyObject(sendReply);

    nanosEnd = get_nanos();

    log2db(
        logdb, "lose-order", 
        std::make_optional<std::string>(std::to_string(carrierID)),
        std::make_optional<std::string>(std::to_string(0)), // usr_state = delivering/losing
        std::nullopt,
        "CARRIER", "server::loseOrderHandler", "INFO", getpid(),
        std::make_optional<unsigned long>(nanosEnd - nanosStart),
        std::nullopt
    );

    // return orderID;

}