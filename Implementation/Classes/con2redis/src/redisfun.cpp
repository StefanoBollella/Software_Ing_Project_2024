#include "con2redis.h"
#include "local.h"
#include <hiredis/read.h>

// RESP2 reply types (see the hiredis documentation)
void print_reply_types() {
  printf("REDIS_REPLY_STRING=%d,\
            REDIS_REPLY_STATUS=%d,\
            REDIS_REPLY_INTEGER=%d,\
            REDIS_REPLY_NIL=%d,\
            REDIS_REPLY_ERROR=%d,\
            REDIS_REPLY_ARRAY=%d\n",
         REDIS_REPLY_STRING, REDIS_REPLY_STATUS, REDIS_REPLY_INTEGER,
         REDIS_REPLY_NIL, REDIS_REPLY_ERROR, REDIS_REPLY_ARRAY);
}

void assertReplyType(redisContext *c, redisReply *r, int type) {
  if (r == NULL)
    dbg_abort("NULL redisReply (error: %s)", c->errstr);
  if (r->type != type) {
    print_reply_types();
    dbg_abort("Expected reply type %d but got type %d\nMessage: %s", type,
              r->type, r->str);
  }
}

// check if there was a reply
void assertReply(redisContext *c, redisReply *r) {
  if (r == NULL)
    dbg_abort("NULL redisReply (error: %s)", c->errstr);
}

void dumpReply(redisReply *r, int indent) {

  sds buffer = sdsempty();

  switch (r->type) {
  case REDIS_REPLY_STRING:
    buffer = sdscatfmt(buffer, "(string) %s\n", r->str);
    break;
  case REDIS_REPLY_STATUS:
    buffer = sdscatfmt(buffer, "(status) %s\n", r->str);
    break;
  case REDIS_REPLY_INTEGER:
    buffer = sdscatfmt(buffer, "(integer) %i\n", r->integer);
    break;
  case REDIS_REPLY_NIL:
    buffer = sdscatlen(buffer, "(nill)\n", 7);
    break;
  case REDIS_REPLY_ERROR:
    buffer = sdscatfmt(buffer, " (error) %s", r->str);
    break;
  case REDIS_REPLY_ARRAY:
    for (size_t i = 0; i < r->elements; i++) {
      dumpReply(r->element[i], indent + 2);
    }
    break;
  default:
    dbg_abort("Don't know how to deal with reply type %d", r->type);
  }

  if (sdslen(buffer) > 0) {
    for (int i = 0; i < indent; i++)
      printf(" ");

    printf("%s", buffer);
  }

  sdsfree(buffer);
}

// Creates a consumer group to a specified stream. Creates a stream if it
// doesn't exists.
void initStreams(redisContext *c, const char *stream, const char *groupName) {
  // diameter is the name of the group
  redisReply *r =
      RedisCommand(c, "XGROUP CREATE %s %s $ MKSTREAM", stream, groupName);
  assertReply(c, r);
  freeReplyObject(r);
}

// checks if stream exists
bool streamExists(redisContext *c, const char *stream) {
  if (c == nullptr)
    dbg_abort("NULL redis context %s ", "when calling streamExists()");

  redisReply *reply = RedisCommand(c, "EXISTS %s", stream);
  assertReply(c, reply);

  bool isExists =
      reply->type == REDIS_REPLY_INTEGER && reply->integer == (long long)1;
  freeReplyObject(reply);

  return isExists;
}
