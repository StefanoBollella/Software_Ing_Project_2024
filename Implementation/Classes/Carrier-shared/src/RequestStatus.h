#ifndef REQ_STAT
#define REQ_STAT

#include <string>

namespace reqstatus {

enum REQ_TYPE {
    UNKNOWN,         // Any unrecognized request type
    GET_ORDS_REQ,    // Carrier orders request
    LOSE_ORD_REQ,    // Carrier lose order request
    DELIVER_ORD_REQ, // Carrier deliver request
    CARRIER_REG      // register carriers in the db
};

// 100-199 Set of success codes
// 200-299 Set of client errors
// 300-399 Set of server errors
enum STATUS_CODE {
    REQ_OK = 100,        // Request was accepted, understood, and acted upon
    BAD_REQ = 200,       // Bad request form, e.g. syntax errors
    NOT_FOUND = 204,     // resource not available
    SERVER_ERR = 300,    // Internal Server Error
    SERVER_DOWN = 301,   // Server can't be reached
    SERVER_FAILURE = 302 // Server can't act on the request
};

typedef struct RequestStatus {
    bool isEmpty = true;
    unsigned char type;
    unsigned short code;
    std::string msg;
} RequestStatus;

static const RequestStatus EmptyReqStatus{ 
    .isEmpty = true,
    .type = 0,
    .code = 0,
    .msg = ""
};

} // namespace reqstatus
  
bool isCleanStatus(reqstatus::RequestStatus& status);

#endif // REQ_STAT