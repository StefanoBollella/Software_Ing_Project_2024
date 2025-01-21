#include "main.h"

#define BUFF_SIZE 100

void registerCustomersHandler(
    redisContext* c2r, redisReply* readReply,
    char* replyStream, Con2DB db, int ithMsg,
    Con2DB logdb
) {

    std::vector<unsigned long> customerIDs;
    char fname[BUFF_SIZE];
    char fval[BUFF_SIZE];
    char msgid[BUFF_SIZE];
    redisReply *sendReply;

    ReadStreamMsgVal(readReply, STREAM_IND, ithMsg,
            req_formats::RCREQ_CUST_QTY_F, fname
    );

    if (strcmp(fname, "cust_qty")) {
        sendReply = RedisCommand(c2r,
            "XADD %s NOMKSTREAM * type %u code %u msgID %s msg %s",
            replyStream, reqstatus::CUST_REGISTER, reqstatus::BAD_REQ,
            msgid, "Bad request, missing cust_qty field"
        );

        assertReplyType(c2r, sendReply, REDIS_REPLY_STRING);
        freeReplyObject(sendReply);
        return;
    }

    ReadStreamMsgVal(readReply, STREAM_IND, ithMsg,
            req_formats::RCREQ_CUST_QTY_V, fval 
    );

    customerIDs = registerCustomersQuery(db, std::stoul(fval));

    sendReply = RedisCommand(c2r,
        "XADD %s NOMKSTREAM * type %u code %u regIDs %s msg %s",
        replyStream, reqstatus::CUST_REGISTER,
        reqstatus::REQ_OK, vecToStr(customerIDs).c_str(),
        "customer registration success"
    );

    assertReplyType(c2r, sendReply, REDIS_REPLY_STRING);
    freeReplyObject(sendReply);
}
