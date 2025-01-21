#include "main.h"

#define BUFF_SIZE 100

void getProductsReqHandler(
    redisContext* c2r, redisReply* readReply,
    char* replyStream, Con2DB db, int ithMsg,
    Con2DB logdb
) {

    char fval[BUFF_SIZE]; 
    std::string msg;
    redisReply* sendReply;
    unsigned nProds;
    std::vector<std::pair<unsigned long, unsigned long>> products;
    int res;

    // reads n_prod
    ReadStreamMsgVal(
        readReply, STREAM_IND, ithMsg,
        req_formats::GPREQ_N_PROD_V, fval
    );
    nProds = std::stoul(fval);
    memset(fval, '\0', sizeof(fval));

    // number of products request is zero
    if (nProds == 0) {
        sendReply = RedisCommand(c2r,
            "XADD %s NOMKSTREAM * type %u "
            "code %u n_prod %u msg %s prodsRcvd []",
            replyStream, reqstatus::PROD_REQ,
            reqstatus::BAD_REQ, nProds, "n_prod > 0 not satisfied"
        );
    }

    // queries the DB and puts the result onto products
    res = getProductsQuery(db, nProds, msg, products); 

    // query success 
    if (res == EXIT_SUCCESS) {
        sendReply = RedisCommand(c2r,
            "XADD %s NOMKSTREAM * type %u "
            "code %u n_prod %u msg %s prodsRcvd %s",
            replyStream, reqstatus::PROD_REQ,
            reqstatus::BAD_REQ, nProds, "get products success",
            vecTuplesToStr(products).c_str()
        );

    } else {
        sendReply = RedisCommand(c2r,
            "XADD %s NOMKSTREAM * type %u "
            "code %u n_prod %u msg %s prodsRcvd []",
            replyStream, reqstatus::PROD_REQ,
            reqstatus::BAD_REQ, nProds, msg.c_str() 
        );
    }

    assertReplyType(c2r, sendReply, REDIS_REPLY_STRING);
    dumpReply(sendReply, 1);
    freeReplyObject(sendReply);
    
}
