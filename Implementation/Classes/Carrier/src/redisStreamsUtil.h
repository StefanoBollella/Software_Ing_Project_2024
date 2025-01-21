#ifndef REDIS_STREAMS_UTIL_H
#define REDIS_STREAMS_UTIL_H

#include "../../Carrier-shared/src/RequestStatus.h"
#include "../../con2redis/src/con2redis.h"
#include "../../con2redis/src/local.h"

bool isValidRedisStream(redisContext* c2r, const char* stream,
                        reqstatus::RequestStatus& status);

#endif // REDIS_STREAMS_UTIL_H
