#include "main.h"

#define BUFF_SIZE 100

unsigned orderHandler(
        redisContext* c2r, redisReply* readReply,
        char* replyStream, Con2DB db, int ithMsg,
        Con2DB logdb
) {
    char fname[BUFF_SIZE], fval[BUFF_SIZE];
    std::string msg;
    redisReply* sendReply;
    std::vector<std::pair<unsigned long, unsigned long>> products;
    int nVals, res;
    unsigned long prodID, qty, customerID, ordID = 0;
    long nanosStart, nanosEnd;
    std::string usr_state;

    nanosStart = get_nanos();

    ReadStreamMsgVal(
        readReply, STREAM_IND, ithMsg,
        req_formats::OREQ_CUST_STATE_V, fval
    );
    usr_state = fval;
    memset(fval, '\0', BUFF_SIZE);

    // reads customerID
    ReadStreamMsgVal(
        readReply, STREAM_IND, ithMsg,
        req_formats::OREQ_CUST_ID_V, fval
    );
    customerID = std::stoul(fval);
    memset(fval, '\0', BUFF_SIZE);

    nVals = ReadStreamMsgNumVal(readReply, STREAM_IND, ithMsg);
    // reads productID and corresponding quantity
    for (int i = req_formats::OREQ_CUST_ID_V; i+2 < nVals; i += 2) {
        ReadStreamMsgVal(
            readReply, STREAM_IND,
            ithMsg, i + 1, fname
        );
        ReadStreamMsgVal(
            readReply, STREAM_IND,
            ithMsg, i + 2, fval
        );

        prodID = std::stoul(fname);
        qty = std::stoul(fval);
        products.emplace_back(std::make_pair(prodID, qty));
        memset(fname, '\0', BUFF_SIZE);
        memset(fval, '\0', BUFF_SIZE);
    }

    // ordID is set by orderQuery
    res = orderQuery(db, customerID, &ordID, products, msg);

    // order query failed
    if (res == EXIT_FAILURE) {
        sendReply = RedisCommand(c2r,
            "XADD %s NOMKSTREAM * type %u customerID %lu "
            "orderID %lu code %u msg %s",
            replyStream, reqstatus::ORDER_REQ, customerID, ordID,
            reqstatus::BAD_REQ, msg.c_str()
        );

    } else {
        sendReply = RedisCommand(c2r,
            "XADD %s NOMKSTREAM * type %u customerID %lu "
            "orderID %lu code %u msg %s",
            replyStream, reqstatus::ORDER_REQ, customerID, ordID,
            reqstatus::REQ_OK, "order confirmed"
        );
    }

    assertReplyType(c2r, sendReply, REDIS_REPLY_STRING);
    dumpReply(sendReply, 1);
    freeReplyObject(sendReply);

    nanosEnd = get_nanos();

    log2db(
        logdb, "order",
        std::make_optional<std::string>(std::to_string(customerID)),
        std::make_optional<std::string>(usr_state),
        std::make_optional<std::string>(std::to_string(ordID)),
        "CUSTOMER", "server::orderHandler", "INFO", getpid(),
        std::make_optional<unsigned long>(nanosEnd - nanosStart),
        std::nullopt
    );

    return res;
}
