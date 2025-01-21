#include "Customer.h"
#include "requestReply.h"
#include "redisStreamsUtil.h"
#include "../../Customer-shared/src/RequestStatus.h"
#include "../../Customer-shared/src/request_formats.h"
#include <iostream>  // std::invalid_argument

#define MIN_NUM_VALUES 4
#define BUFF_SIZE 1000

namespace customer {
/*
 * Orders the customer's order with it's related products by sending
 * an order request to a customer server through a redis stream.
 * Returns EXIT_SUCCESS on success, else EXIT_FAILURE.
 *
 * Forms of request and response:
 * request: type {req_type} customerID {id} {prod_id} {prod_qty}
 *          [{prod_id} {prod_qty} ...].
 * response: type {req_type} customerID {id} orderID {ord_id} code {status_code} msg {msg_body}
 *
 * The syntax structure is field value [field value ...]
 */
int Customer::order(
    redisContext *c2r, const char* requestStream,
    const char* replyStream,
    std::unique_ptr<std::unordered_map<ulong, ulong>>& products,
    reqstatus::RequestStatus& status,
    const char* consumerGrp
) {

    if (c2r == nullptr)
        throw std::invalid_argument(
            "order(): Err: invalid argument: redis context is null"
        );

    if (requestStream == nullptr)
        throw std::invalid_argument(
            "order(): Err: invalid argument: request stream is null"
        );

    if (replyStream == nullptr)
        throw std::invalid_argument(
            "order(): Err: invalid argument: reply stream is null"
        );

    if (!isCleanStatus(status))
        throw std::invalid_argument(
            "order(): Err: invalid argument: request status is dirty"
        );

    if (products->empty())
        throw std::invalid_argument(
            "order(): Err: invalid argument: products is empty"
        );

    if (!isValidRedisStream(c2r, requestStream, status))
        throw std::invalid_argument(
            std::string(__func__) + "(): " + status.msg
        );

    if (!isValidRedisStream(c2r, replyStream, status))
        throw std::invalid_argument(
            std::string(__func__) + "(): " + status.msg
        );

    char buff[BUFF_SIZE];
    redisReply *reply;
    ulong ordID, reqCodeRcvd;
    unsigned char reqType;
    const unsigned short MAX_SERVER_STAT_CODE = 399;
    status = reqstatus::EmptyReqStatus;

    status.type = reqstatus::ORDER_REQ;
    status.isEmpty = false;

    if (this->orders.size() >= this->orderLimit) {
        status.code = reqstatus::BAD_REQ;
        status.msg = "Customer: " + std::to_string(this->id) +
                     " exceeded its limit";
        return EXIT_FAILURE;
    }

    // ---------------------------- SEND REQUEST -----------------------------
    snprintf(
        buff, sizeof(buff),
        "XADD %s NOMKSTREAM * type %u usr_state %u customerID %lu",
        requestStream, reqstatus::ORDER_REQ, this->currentState, this->id
    );

    std::string orderReq(buff);
    for (const std::pair<const ulong, ulong> &prod : (*products))
        orderReq += " " + std::to_string(prod.first) + " " +
                    std::to_string(prod.second);

    // orders to customer server
    reply = RedisCommand(c2r, orderReq.c_str());

    if (!isValidXAddReply(c2r, reply, status)) {
        freeReplyObject(reply);
        return EXIT_FAILURE;
    }

    freeReplyObject(reply);

    // ---------------------------- READ REPLY -------------------------------
    // waits for a reply for 5 seconds
    // reads 2 messages to make sure that old pending message
    // is also read from the stream; this is helpful when
    // reusing the same redis streams to make sure that the
    // client can read the correct response
    reply = RedisCommand(
        c2r, "XREADGROUP GROUP %s Alice COUNT 2 BLOCK 5000 NOACK STREAMS %s >",
        consumerGrp, replyStream
    );


    if (!isValidXReadGroupReply(
        c2r, reply, status, consumerGrp)) {
        freeReplyObject(reply);
        return EXIT_FAILURE;
    }

    int numMsg = ReadStreamNumMsg(reply, 0);
    int lstMsg = numMsg - 1;

    if (!isValidReplyMsgNumVals(
        c2r, reply, status,
        req_formats::GPREP_PRODS_RCVD_V + 1, lstMsg)) {
        freeReplyObject(reply);
        return EXIT_FAILURE;
    }

    dumpReply(reply, 1);

    // process the reply from server
    reqType = readReqType(reply, status, lstMsg);
    if (reqType != reqstatus::ORDER_REQ) {
        status.isEmpty = false;
        status.type = reqType;
        status.code = reqstatus::SERVER_ERR;
        status.msg = std::string(
            "Customer server error: got a reply, "
            "but it's request type is different!");
        freeReplyObject(reply);
        return EXIT_FAILURE;
    }

    reqCodeRcvd = readReqStatusCode(reply, status, lstMsg);
    // checks if reqCode is a server error
    if (reqCodeRcvd >= reqstatus::SERVER_ERR &&
        reqCodeRcvd <= MAX_SERVER_STAT_CODE) {
        freeReplyObject(reply);
        return EXIT_FAILURE;
    }

    memset(buff, '\0', sizeof(buff));
    ReadStreamMsgVal(reply, 0, 0, req_formats::OREP_ORD_ID_V, buff);
    ordID = std::stoul(buff);

    freeReplyObject(reply);
    // ------------------------ UPDATE CUSTOMER STATE ------------------------

    // updates this customer's order list (see Customer.h for it's purpose)
    if (reqCodeRcvd == reqstatus::REQ_OK) {
        this->orders.push_back(ordID);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

}  // namespace customer
