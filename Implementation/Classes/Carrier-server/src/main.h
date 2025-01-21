#ifndef main_h
#define main_h

#include <iostream>
#include <cassert>
#include <unistd.h>
#include <csignal>
#include <string>
#include <vector>

// redis stuff
#include "../../con2redis/src/con2redis.h"
#include "../../con2redis/src/local.h"

// clock stuff
#include "../../Clock/src/clock.h"

#include "../../Carrier-shared/src/RequestStatus.h"
#include "../../Carrier-shared/src/request_formats.h"
#include "../../Carrier-shared/src/utils.h"

#define STREAM_IND 0

// PostgreSQL stuff
#include "../../con2db/pgsql.h"

#include "../../logger/src/log2db.h"

int parse_request(redisReply* reply, unsigned long kthStream, int ithMsg);

void deliverOrdHandler(
        redisContext* c2r, redisReply* readReply,
        char* replyStream, Con2DB db, int ithMsg, Con2DB logdb
);

int deliverOrdQuery(
    Con2DB db,
    unsigned long carrierID,
    unsigned long orderID,
    std::string &msg
);

void loseOrdHandler(
        redisContext* c2r, redisReply* readReply,
        char* replyStream, Con2DB db, int ithMsg, Con2DB logdb
);

int loseOrdQuery(
    Con2DB db,
    unsigned long carrierID,
    unsigned long orderID,
    std::string &msg
);

void getOrdersHandler(
        redisContext* c2r, redisReply* readReply,
        char* replyStream, Con2DB db, int ithMsg, Con2DB logdb
);

std::vector<unsigned long> getOrdersQuery(
    Con2DB db,
    int ordsNum,
    std::string &msg,
    int *funcRes,
    unsigned long carrierID
);

void registerCarriersHandler(
        redisContext* c2r, redisReply* readReply,
        char* replyStream, Con2DB db, int ithMsg, Con2DB logdb    
);

std::vector<unsigned long> registerCarriersQuery(
    Con2DB db, int quantity
);

void notifyActivity(redisContext *c2r, const char* req_pong_supplier, const char* reply_ping);

#endif 
