#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <cassert>   // assert
#include <unistd.h>  // getopt
#include <csignal>   // signal, SIGINT
#include <string>    // memset
#include <vector>
#include <cstdlib>   // strtol

// REDIS LIBS
#include "../../con2redis/src/con2redis.h"
#include "../../con2redis/src/local.h"


#include "../../Customer-shared/src/RequestStatus.h"
#include "../../Customer-shared/src/request_formats.h"
#include "../../Customer-shared/src/utils.h"
// CLOCK LIBS
#include "../../Clock/src/clock.h"

#define STREAM_IND 0

#include "../../logger/src/log2db.h"
// PostgreSQL LIBS
#include "../../con2db/pgsql.h"


int parse_request(redisReply* reply, unsigned long kthStream, int ithMsg);

void registerCustomersHandler(
    redisContext* c2r, redisReply* readReply,
    char* replyStream, Con2DB db, int ithMsg,
    Con2DB logdb
);

std::vector<unsigned long> registerCustomersQuery(Con2DB db, int qty);

unsigned orderHandler(
    redisContext* c2r, redisReply* readReply,
    char* replyStream, Con2DB db, int ithMsg,
    Con2DB logdb
);

int orderQuery(
    Con2DB db,
    unsigned long custID,
    unsigned long *ordID,
    std::vector<std::pair<unsigned long, unsigned long>> &products,
    std::string &msg
);

unsigned cancelOrderHandler(
    redisContext* c2r, redisReply* readReply,
    char* replyStream, Con2DB db, int ithMsg,
    Con2DB logdb
);

int cancelOrderQuery(
    Con2DB db,
    unsigned long custID,
    unsigned long orderID,
    std::string &msg
);

void getProductsReqHandler(
    redisContext* c2r, redisReply* readReply,
    char* replyStream, Con2DB db, int ithMsg,
    Con2DB logdb
);

int getProductsQuery(
    Con2DB db, unsigned numProds, std::string &msg,
    std::vector<std::pair<unsigned long, unsigned long>> &products
);

void notifyActivity(
    redisContext *c2r, const char *req_pong_stream,
    const char *reply_ping
);

#endif
