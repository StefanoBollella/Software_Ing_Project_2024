#include "main.h"

int parse_request(redisReply* reply, unsigned long kthStream, int ithMsg) {

    char fname[100];
    char fval[100];
    ReadStreamMsgVal(reply, kthStream, ithMsg, 0, fname);
    if (strncmp(fname, "type", 4)) {
        std::cerr << __func__ << ": request is malformed, missing type field" 
                  << std::endl;
        return -1;
    }
    ReadStreamMsgVal(reply, kthStream, ithMsg, 1, fval);
    return atoi(fval);
}
