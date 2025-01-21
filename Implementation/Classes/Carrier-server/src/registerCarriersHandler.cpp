#include "main.h"

#define BUFF_SIZE 100

void registerCarriersHandler(
    redisContext* c2r, redisReply* readReply, char* replyStream, Con2DB db, int ithMsg, Con2DB logdb    
){
    std::vector<unsigned long> carrierIDs;
    char fname[BUFF_SIZE];
    char fval[BUFF_SIZE];
    char msgid[BUFF_SIZE];

    redisReply *sendReply;

    ReadStreamMsgVal(readReply, STREAM_IND, ithMsg, req_formats::RCARREQ_QTY_F, fname);

    if (strcmp(fname, "quantity")) {
        sendReply = RedisCommand(c2r,
            "XADD %s NOMKSTREAM * type %u code %u msgID %s msg %s",
            replyStream, reqstatus::CARRIER_REG, reqstatus::BAD_REQ,
            msgid, "Bad request, missing quantity field"
        );

        assertReplyType(c2r, sendReply, REDIS_REPLY_STRING);
        freeReplyObject(sendReply);
        return;
    }

    ReadStreamMsgVal(readReply, STREAM_IND, ithMsg, req_formats::RCARREQ_QTY_V, fval);

    carrierIDs = registerCarriersQuery(db, std::stoul(fval));

    sendReply = RedisCommand(c2r,
        "XADD %s NOMKSTREAM * type %u code %u carrIDs %s msg %s",
        replyStream, reqstatus::CARRIER_REG,
        reqstatus::REQ_OK, vecToStr(carrierIDs).c_str(),
        "carrier registration success"
    );

    assertReplyType(c2r, sendReply, REDIS_REPLY_STRING);
    freeReplyObject(sendReply);
}