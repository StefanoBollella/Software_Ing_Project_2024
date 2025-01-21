#include "Customer.h"
#include "requestReply.h"
#include "redisStreamsUtil.h"
#include "../../Customer-shared/src/RequestStatus.h"
#include "../../Customer-shared/src/request_formats.h"
#include <iostream>

#define RAND_CANCEL -1
#define MIN_NUM_VALUES 8
#define BUFF_SIZE 100

namespace customer {

typedef std::mt19937 rng_type;
std::random_device rand_dev;  // seed
rng_type random_gen(rand_dev());

/**
 * Sends a cancel order request to the customer server.
 * orderInd is equal to -1  by default which means
 * that this will select any pending order to cancel, see Customer.h.
 * Returns EXIT_SUCCESS if a request was sent, else EXIT_FAILURE.
 *
 * Forms of request and response: 
 *
 * request: type {req_type} customerID {id} orderID {ord_id}
 * response: type {req_type} customerID {id} code {status_code} msg {msg_body}
 *
 * The syntax structure is field value [field value ...]
 */
int Customer::cancelOrder(
    redisContext *c2r, const char *requestStream,
    const char *replyStream, reqstatus::RequestStatus& status,
    const char *consumerGrp, int orderInd
) {
    if (c2r == nullptr)
        throw std::invalid_argument(
            "cancelOrder(): Err: invalid argument: redis context is null"
        );

    if (requestStream == nullptr)
        throw std::invalid_argument(
            "cancelOrder(): Err: invalid argument: request stream is null"
        );

    if (replyStream == nullptr)
        throw std::invalid_argument(
            "cancelOrder(): Err: invalid argument: reply stream is null"
        );

    if (!isCleanStatus(status))
        throw std::invalid_argument(
            "cancelOrder(): Err: invalid argument: request status is dirty"
        );


    ulong reqCodeRcvd;
    unsigned char reqType;
    redisReply *reply;
    char buff[BUFF_SIZE];
    const unsigned short MAX_SERVER_STAT_CODE = 399;

    // will generate a random pending order to cancel
    if (orderInd == RAND_CANCEL) {
        std::uniform_int_distribution<rng_type::result_type>
            uDistRandOrderToCancel(0, this->orders.size() - 1);

        orderInd = uDistRandOrderToCancel(random_gen);
    }

    if (!isValidRedisStream(c2r, requestStream, status))
        throw std::invalid_argument(
            std::string(__func__) + "(): " + status.msg
        );

    if (!isValidRedisStream(c2r, replyStream, status))
        throw std::invalid_argument(
            std::string(__func__) + "(): " + status.msg
        );

    // ---------------------------- SEND REQUEST -----------------------------
    if (orderInd < this->orders.size()) {
        // send cancel order with id = orderID request to customer server
        reply =
            RedisCommand(
                c2r,
                "XADD %s NOMKSTREAM * type %u usr_state %u "
                "customerID %lu orderID %lu",
                requestStream, reqstatus::CANC_ORDER_REQ, this->currentState,
                this->id, this->orders[orderInd]
            );

        if (!isValidXAddReply(c2r, reply, status)) {
            freeReplyObject(reply);
            return EXIT_FAILURE;
        }

    // ---------------------------- READ REPLY -------------------------------
        // reads 2 messages to make sure that old pending message
        // is also read from the stream; this is helpful when
        // reusing the same redis streams to make sure that the
        // client can read the correct response
        reply =
            RedisCommand(
                c2r, "XREADGROUP GROUP %s Alice COUNT 2 "
                "BLOCK 5000 NOACK STREAMS %s >",
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
            MIN_NUM_VALUES, lstMsg)) {
            freeReplyObject(reply);
            return EXIT_FAILURE;
        }

        dumpReply(reply, 1);

        // process the reply from server
        reqType = readReqType(reply, status, lstMsg);
        if (reqType != reqstatus::CANC_ORDER_REQ) {
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

        if (reqCodeRcvd == reqstatus::REQ_OK) {
            this->orders.erase(this->orders.begin() + orderInd);
            return EXIT_SUCCESS;

        } else if (reqCodeRcvd == reqstatus::BAD_REQ) {
            // BAD_REQ can mean that order to cancel is already in traveling
            // so erase this pending order
            this->orders.erase(this->orders.begin() + orderInd);
            return EXIT_FAILURE;
        }
    }  // end if orderInd < this->orders.size()
    return EXIT_FAILURE;
}

}  // namespace customer
