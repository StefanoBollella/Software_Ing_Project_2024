#include "redisStreamsUtil.h"

/* checks that the redis stream exists, and changes the request status
 * accordingly */
bool isValidRedisStream(redisContext *c2r, const char *stream,
                        reqstatus::RequestStatus &status) {

    if (!streamExists(c2r, stream)) {
        status.isEmpty = false;
        status.code = reqstatus::BAD_REQ;
        status.msg = std::string("Redis stream: ") + stream + " not found";
        return false;
    }
    return true;
}

