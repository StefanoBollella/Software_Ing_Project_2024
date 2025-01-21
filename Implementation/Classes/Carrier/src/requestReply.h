#ifndef REQUEST_REPLY_H
#define REQUEST_REPLY_H

#include "../../Carrier-shared/src/RequestStatus.h"
#include "../../con2redis/src/con2redis.h"
#include "../../con2redis/src/local.h"

bool isValidXReadGroupReply(redisContext* c2r, redisReply* reply,
                            reqstatus::RequestStatus& status,
                            const char* consumerGrp);

bool isValidReplyMsgNumVals(redisContext *c2r, redisReply *reply,
                            reqstatus::RequestStatus &status,
                            unsigned char minNumVals, int ithMsg);

bool isValidXAddReply(redisContext* c2r, redisReply* reply,
                      reqstatus::RequestStatus& status);

unsigned char readReqType(redisReply* reply, reqstatus::RequestStatus& status, int ithMsg);

unsigned short readReqStatusCode(redisReply* reply,
                                 reqstatus::RequestStatus& status, int ithMsg);


#endif // REQUEST_REPLY_H
