#ifndef con2redis_h

#define con2redis_h

#include <stdio.h>
#include <stdlib.h>

// include hiredis
extern "C" {
#include <hiredis/hiredis.h>
}

// #include "global.h"
// #include "../../con2db/pgsql.h"


#define dbg_log(fmt, ... )			\
    do { \
        fprintf(stderr, "%s:%d : " fmt "\n", __FILE__, __LINE__,__VA_ARGS__); \
    } while (0);


#define dbg_abort(fmt, ...) \
    do { \
        dbg_log(fmt, __VA_ARGS__); exit(-1); \
    } while (0)


#define RedisCommand(fmt, ...)			\
  (redisReply*) redisCommand(fmt, __VA_ARGS__)


// ------------------ redisfun.cpp ------------------

void assertReplyType(redisContext *c, redisReply *r, int type);

void assertReply(redisContext *c, redisReply *r);

void dumpReply(redisReply *r, int indent);

void initStreams(redisContext *c, const char *stream, const char *consumerGroup);

bool streamExists(redisContext *c, const char *stream);

// ------------------ readreply.cpp ------------------

// only implemented in "old" directory
//void ReadStreamReply(redisReply *r, int indent, int readstatus);

// The following functions requires that the reply type is REDIS_REPLY_ARRAY
// use assertReplyType(...) before calling them.
int ReadStreamName(redisReply *r, char *streamname, long unsigned int k);

int ReadNumStreams(redisReply *r);

int ReadStreamNumMsg(redisReply *r, long unsigned int streamnum);

int ReadStreamNumMsgID(redisReply *r, long unsigned int streamnum, int msgnum, char *msgid);

int ReadStreamMsgNumVal(redisReply *r, long unsigned int streamnum, int msgnum);

int ReadStreamMsgVal(redisReply *r, long unsigned int streamnum, int msgnum, int entry, char *fval);


#endif
