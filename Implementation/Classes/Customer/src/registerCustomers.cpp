#include "main.h"
#include "requestReply.h"

#define MIN_NUM_VALUES 6
#define FNAME_LEN 50
#define FVAL_LEN 1000


/*
 * Registers customers with their IDs by sending registration
 * request to a customer server via a redis stream.
 *
 * Forms of request and response:
 * request: type {req_type} cust_qty {qty}
 * response: type {req_type} code {status_code} regIDs {[x1, ..., xn]} msg {msg}
 *
 * @param customerIDs contains all the IDs of the customers to register
 * @param unregIDs contains all the IDs that failed to be registered
 *
 */
int registerCustomers(redisContext *c2r, const char *requestStream,
                      const char *replyStream, const char* consumerGrp,
                      reqstatus::RequestStatus &status, unsigned cust_qty,
                      std::vector<ulong> &regIDs) {

    if (c2r == nullptr)
        throw std::invalid_argument(
            "registerCustomers(): Err: invalid argument: redis context is null");

    if (requestStream == nullptr)
        throw std::invalid_argument(
            "registerCustomers(): Err: invalid argument: request stream is null");

    if (replyStream == nullptr)
        throw std::invalid_argument(
            "registerCustomers(): Err: invalid argument: reply stream is null");

    if (!isCleanStatus(status))
        throw std::invalid_argument(
            "registerCustomers(): Err: invalid argument: request status is dirty");

    char fname[FNAME_LEN];
    char fval[FVAL_LEN];

    status.type = reqstatus::CUST_REGISTER;
    status.isEmpty = false;

    // ---------------------------- SEND REQUEST -----------------------------
    redisReply *reply = RedisCommand(c2r,
            "XADD %s NOMKSTREAM * type %u cust_qty %u",
            requestStream, reqstatus::CUST_REGISTER,
            cust_qty
    );

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
        consumerGrp, replyStream);

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
    unsigned char reqType = readReqType(reply, status, lstMsg);
    if (reqType != reqstatus::CUST_REGISTER) {
        status.isEmpty = false;
        status.type = reqType;
        status.code = reqstatus::SERVER_ERR;
        status.msg = std::string(
            "Customer server error: got a reply, "
            "but it's request type is different!");
        freeReplyObject(reply);
        return EXIT_FAILURE;
    }

    const unsigned short MAX_SERVER_STAT_CODE = 399;
    ulong reqCodeRcvd = readReqStatusCode(reply, status, lstMsg);
    // checks if reqCode is a server error
    if (reqCodeRcvd >= reqstatus::SERVER_ERR &&
        reqCodeRcvd <= MAX_SERVER_STAT_CODE) {
        freeReplyObject(reply);
        return EXIT_FAILURE;
    }

    // check the IDs of those failed to register, for logging
    ReadStreamMsgVal(reply, 0, 0, req_formats::RCREP_REG_IDS_F, fname);
    if (strcmp(fname, "regIDs")) {
        status.code = reqstatus::SERVER_ERR;
        status.msg = std::string(
            "Malformed server response: expected regIDs, got ")
            + fname + " instead";
        freeReplyObject(reply);
        return EXIT_FAILURE;
    }
    ReadStreamMsgVal(reply, 0, 0, req_formats::RCREP_REG_IDS_V, fval);
    regIDs = strToULVector(fval);

    memset(fname, '\0', FNAME_LEN);
    memset(fval, '\0', FVAL_LEN);

    // checks the msg of the response
    ReadStreamMsgVal(reply, 0, 0, req_formats::RCREP_MSG_F, fname);
    if (strcmp(fname, "msg")) {
        status.code = reqstatus::SERVER_ERR;
        status.msg = std::string(
            "Malformed server response: expected msg, got ")
            + fname + " instead";
        freeReplyObject(reply);
        return EXIT_FAILURE;
    }

    ReadStreamMsgVal(reply, 0, 0, req_formats::RCREP_MSG_V, fval);
    status.msg = std::string(fval);

    freeReplyObject(reply);

    return EXIT_SUCCESS;
}
