#include "main.h"

#define BUFF_SIZE 100

unsigned cancelOrderHandler(
        redisContext* c2r, redisReply* readReply,
        char* replyStream, Con2DB db, int ithMsg,
        Con2DB logdb
) {
    
    char fval[BUFF_SIZE];
    redisReply* sendReply;
    unsigned long orderID;
    std::string msg;
    int resQuery;
    long nanosStart, nanosEnd;
    std::string usr_state, customerID;

    nanosStart = get_nanos();
    
    ReadStreamMsgVal(
        readReply, STREAM_IND, ithMsg,
        req_formats::COREQ_CUST_STATE_V, fval
    );
    usr_state = fval;
    memset(fval, '\0', BUFF_SIZE);

    // reads customerID
    ReadStreamMsgVal(
        readReply, STREAM_IND, ithMsg,
        req_formats::COREQ_CUST_ID_V, fval
    );
    customerID = fval;
    memset(fval, '\0', sizeof(fval));

    // reads orderID
    ReadStreamMsgVal(
        readReply, STREAM_IND, ithMsg,
        req_formats::COREQ_ORD_ID_V, fval
    );
    orderID = std::stoul(fval);
    memset(fval, '\0', sizeof(fval));


    resQuery = cancelOrderQuery(
        db, std::stoul(customerID.c_str()), orderID, msg
    );
    if (resQuery == EXIT_SUCCESS) {
        sendReply = RedisCommand(c2r,
            "XADD %s NOMKSTREAM * type %u customerID %s "
            "orderID %lu code %u msg %s",
            replyStream, reqstatus::CANC_ORDER_REQ,
            customerID.c_str(), orderID,
            reqstatus::REQ_OK, "order cancellation confirmed"
        );
    } else {
        sendReply = RedisCommand(c2r,
            "XADD %s NOMKSTREAM * type %u customerID %lu "
            "orderID %lu code %u msg %s",
            replyStream, reqstatus::CANC_ORDER_REQ,
            customerID.c_str(), orderID,
            reqstatus::BAD_REQ, msg.c_str()
        );
    }

    assertReply(c2r, sendReply);
    assertReplyType(c2r, sendReply, REDIS_REPLY_STRING);
    freeReplyObject(sendReply);

    nanosEnd = get_nanos();

    log2db(
        logdb, "cancel-order", std::make_optional<std::string>(customerID),
        std::make_optional<std::string>(usr_state),
        std::make_optional<std::string>(std::to_string(orderID)),
        "CUSTOMER", "server::orderHandler", "INFO", getpid(),
        std::make_optional<unsigned long>(nanosEnd - nanosStart),
        std::nullopt
    );

    return orderID;
}
