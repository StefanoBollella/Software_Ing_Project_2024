#include <iostream>
#include <memory>
#include "ProductWindow.h"
#include "requestReply.h"
#include "redisStreamsUtil.h"

#include "../../Customer-shared/src/RequestStatus.h"
#include "../../Customer-shared/src/request_formats.h"
#include "../../Customer-shared/src/utils.h"

#define MAX_N_PRODUCTS 20
#define EXPECTED_N_CONSUMER_STREAMS 1
#define BUFF_SIZE 500
#define STREAM_IND 0
#define MSG_IND 0

/* checks that the max number of products requested is not over MAX_N_PRODUCTS.
 * Modifies the request status accordingly if the amount is over. Otherwise
 * returns true.
 *
 */
bool isValidMaxNewProducts(unsigned char maxNewProducts,
                           reqstatus::RequestStatus &status) {

    if (maxNewProducts > MAX_N_PRODUCTS) {
        status.isEmpty = false;
        status.code = reqstatus::BAD_REQ;
        status.msg = "Number of requested products exceeded limit\n";
        status.msg =
            status.msg + "Expected limit: " + std::to_string(MAX_N_PRODUCTS);
        return false;
    }
    return true;
}

/**
 * Requests some products from the customer server via redis streams: reqStream
 * and replyStream. The number of new products requested must be at most
 * maxNewProducts.
 *
 * The products are then stored in prodWindow with their respective quantities.
 *
 * The request status is updated accordingly based on the request and response
 * from the customer server.
 *
 * Forms of request and response:
 * request: type {req_type} n_prod {number_products_requested}
 * response: type {req_type} code {status_code} n_prod
 * {number_products_requested} [{prod_id} {prod_qty} ...]
 *
 * @param c2r            Redis context, which stores the state of the redis
 * stream connection.
 * @param reqStream      Request redis stream.
 * @param replyStream    Reply redis stream, where server reply arrives.
 * @param maxNewProducts Max number of new products that we can expect to
 * receive.
 * @param prodWindow     ProductWindow object, where we store new products.
 * @param status         Request status data structure that contains the request
 * type, status code, and eventual message from the server reply or from an
 * error found during the request process.
 *
 * @throw: std::invalid_argument
 * @throw: std::exception
 * @return: EXIT_FAILURE if there was an error, otherwise EXIT_SUCCESS 
 *
 */
int getProducts(redisContext *c2r, const char *reqStream,
                  const char *replyStream, unsigned char maxNewProducts,
                  std::unique_ptr<prodwindow::ProductWindow> &prodWindow,
                  reqstatus::RequestStatus &status,
                  const char* consumerGrp) noexcept(false) {

    if (c2r == nullptr)
        throw std::invalid_argument(
                "getProducts(): Err: invalid argument: redis context is null");

    if (reqStream == nullptr)
        throw std::invalid_argument(
            "getProducts(): Err: invalid argument: request stream is null");

    if (replyStream == nullptr)
        throw std::invalid_argument(
            "getProducts(): Err: invalid argument: reply stream is null");

    if (!isCleanStatus(status))
        throw std::invalid_argument("getProducts(): Err: invalid argument: "
                                    "request status is dirty");
    if (prodWindow == nullptr)
        throw std::invalid_argument(
            "getProducts(): Err: invalid argument: product window is null");

    status.type = reqstatus::PROD_REQ;
    if (!isValidMaxNewProducts(maxNewProducts, status)) {
        throw std::invalid_argument(
            std::string(__func__) + "(): " + status.msg
        );
    }
    if (!isValidRedisStream(c2r, reqStream, status)) {
        throw std::invalid_argument(
            std::string(__func__) + "(): " + status.msg
        );
    }
    if (!isValidRedisStream(c2r, replyStream, status)) {
        throw std::invalid_argument(
            std::string(__func__) + "(): " + status.msg
        );
    }

    redisReply *reply;
    char strTuples[BUFF_SIZE];
    std::vector<std::pair<ulong, long>> tuplesVec;

    // ---------------------------- SEND REQUEST ------------------------------
    reply = RedisCommand(
        c2r, "XADD %s NOMKSTREAM * type %u n_prod %u", reqStream,
        reqstatus::PROD_REQ, maxNewProducts
    );

    if (!isValidXAddReply(c2r, reply, status)) {
        freeReplyObject(reply);
        return EXIT_FAILURE;
    }

    // ---------------------------- READ REPLY --------------------------------
    // waits for a reply for 5 seconds
    // reads 2 messages to make sure that old pending message
    // is also read from the stream; this is helpful when reusing the same redis
    // streams to make sure that the client can read the correct response
    reply = RedisCommand(
        c2r, "XREADGROUP GROUP %s Alice COUNT 2 BLOCK 5000 STREAMS %s >",
        consumerGrp, replyStream
    );

    // EXIT_FAILURE if SERVER_DOWN or
    // SERVER_ERR (number of values in the first message is not incorrect)
    // See isValidXReadGroupReply to find out more
    if (!isValidXReadGroupReply(
        c2r, reply, status, consumerGrp)) {
        freeReplyObject(reply);
        return EXIT_FAILURE;
    }

    // isValidXReadGroupReply already makes sure that numMsg is positive
    int numMsg = ReadStreamNumMsg(reply, 0);
    int lstMsg = numMsg - 1;

    if (!isValidReplyMsgNumVals(
        c2r, reply, status,
        req_formats::GPREP_PRODS_RCVD_V + 1, lstMsg)) {
        freeReplyObject(reply);
        return EXIT_FAILURE;
    }

    dumpReply(reply, 1);

    unsigned char reqType = readReqType(reply, status, lstMsg);
    if (reqType != reqstatus::PROD_REQ) {
        status.isEmpty = false;
        status.type = reqType;
        status.code = reqstatus::SERVER_ERR;
        status.msg = std::string(
            "Customer server error: got a reply, "
            "but it's request type is different!");
        freeReplyObject(reply);
        return EXIT_FAILURE;
    }

    unsigned short MAX_SERVER_STAT_CODE = 399;
    ulong reqCodeRcvd = readReqStatusCode(reply, status, lstMsg);
    // checks if reqCode is a server error
    if (reqCodeRcvd >= reqstatus::SERVER_ERR &&
        reqCodeRcvd <= MAX_SERVER_STAT_CODE) {
        freeReplyObject(reply);
        return EXIT_FAILURE;
    }

    ulong numProdsRcvd = readReqNumProds(reply, status, lstMsg);
    if (numProdsRcvd == 0) {
        freeReplyObject(reply);
        return EXIT_SUCCESS;
    }

    ReadStreamMsgVal(
        reply, STREAM_IND, MSG_IND,
        req_formats::GPREP_PRODS_RCVD_V, strTuples
    );

    tuplesVec = strTuplesToVec(strTuples);
    for (std::pair<ulong, long> p: tuplesVec) {
        if (p.second == 0)
            continue;
        // inserts or updates products
        prodWindow->insertProduct(p.first, p.second);
    }

    freeReplyObject(reply);

    return EXIT_SUCCESS;
}
