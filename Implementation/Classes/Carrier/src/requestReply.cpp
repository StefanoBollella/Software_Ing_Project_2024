#include "requestReply.h" 
#include "../../Carrier-shared/src/request_formats.h"

#define BUFF_SIZE 50
#define TYPE_F_IND 0
#define TYPE_V_IND 1

/* checks that the field 'type' exists in the request stored in reply.
 * If it doesn't exists then modify the request status to reflect the error,
 * else read and return it's value.
 */
unsigned char readReqType(redisReply* reply,
                          reqstatus::RequestStatus& status, int ithMsg) {
    char fieldName[BUFF_SIZE];
    char fieldVal[BUFF_SIZE];

    // reads the field 'type'
    ReadStreamMsgVal(reply, 0, ithMsg, TYPE_F_IND, fieldName);
    // checks if field 'type' is present
    if (strcmp("type", fieldName) != 0) {
        status.isEmpty = false;
        status.type = reqstatus::UNKNOWN;
        status.code = reqstatus::SERVER_ERR;
        status.msg =
            std::string("Carrier server error: expected 'type' field, got '") +
            fieldName + "' instead";
        return reqstatus::UNKNOWN;
    }

    // reads the value of 'type'
    ReadStreamMsgVal(reply, 0, ithMsg, TYPE_V_IND, fieldVal);
    status.type = std::stoul(fieldVal);
    return status.type;
}

/* checks that the field 'code' exists in the request stored in reply.
 * If it doesn't exists then modify the request status to reflect the error,
 * else read and return it's value.
 */
unsigned short readReqStatusCode(redisReply* reply,
                                 reqstatus::RequestStatus& status, int ithMsg) {
    char fieldName[BUFF_SIZE];
    char fieldVal[BUFF_SIZE];

    // reads the field 'code'
    if (status.type == reqstatus::GET_ORDS_REQ)
        ReadStreamMsgVal(reply, 0, ithMsg, req_formats::GORDREP_CODE_F, fieldName);
    else if (status.type == reqstatus::LOSE_ORD_REQ)
        ReadStreamMsgVal(reply, 0, ithMsg, req_formats::LREP_CODE_F, fieldName);
    else if (status.type == reqstatus::DELIVER_ORD_REQ)
        ReadStreamMsgVal(reply, 0, ithMsg, req_formats::DREP_CODE_F, fieldName);
    else if (status.type == reqstatus::CARRIER_REG)
        ReadStreamMsgVal(reply, 0, ithMsg, req_formats::RCARREP_CODE_F, fieldName);
    else if (status.type == reqstatus::UNKNOWN)
        dbg_abort("%s: request type found in status is UNKNOWN", __func__);

    // checks if field 'code' is present
    if (strcmp("code", fieldName) != 0) {
        status.isEmpty = false;
        status.code = reqstatus::SERVER_ERR;
        status.msg =
            std::string("Carrier server error: expected 'code' field, got '") +
            fieldName + "' instead";
        return reqstatus::SERVER_ERR;
    }

    // reads the value of 'code'
    if (status.type == reqstatus::GET_ORDS_REQ)
        ReadStreamMsgVal(reply, 0, ithMsg, req_formats::GORDREP_CODE_V, fieldVal);
    else if (status.type == reqstatus::LOSE_ORD_REQ)
        ReadStreamMsgVal(reply, 0, ithMsg, req_formats::LREP_CODE_V, fieldVal);
    else if (status.type == reqstatus::DELIVER_ORD_REQ)
        ReadStreamMsgVal(reply, 0, ithMsg, req_formats::DREP_CODE_V, fieldVal);
    else if (status.type == reqstatus::CARRIER_REG)
        ReadStreamMsgVal(reply, 0, ithMsg, req_formats::RCARREP_CODE_V, fieldVal);

    status.code = (unsigned short)std::stoi(fieldVal);
    return status.code;
}

/* checks that the XADD redis reply is valid. It changes the
 * the request status accordingly */
bool isValidXAddReply(redisContext* c2r, redisReply* reply,
                      reqstatus::RequestStatus& status) {

    assertReply(c2r, reply);

    if (reply->type == REDIS_REPLY_NIL || reply->type == REDIS_REPLY_ERROR) {
        status.isEmpty = false;
        status.code = reqstatus::SERVER_DOWN;
        status.msg =
            "Carrier server error: server timeout, no response from server";
        status.msg = status.msg + "\n | redis stream doesn't exists";
        return false;
    }

    assertReplyType(c2r, reply, REDIS_REPLY_STRING);

    return true;
}

/* checks that the XREADGROUP/XREAD redis reply is valid, and changes 
 * the request status accordingly */
bool isValidXReadGroupReply(redisContext* c2r, redisReply* reply,
                            reqstatus::RequestStatus& status,
                            const char* carrierGrp) {

    assertReply(c2r, reply);
    // checks if server timeout occurred, so no response was received
    // reply->type == REDIS_REPLY_ARRAY otherwise
    if (reply->type == REDIS_REPLY_NIL) {
        status.isEmpty = false;
        status.code = reqstatus::SERVER_DOWN;
        status.msg =
            "Carrier server error: server timeout, no response from server";
        status.msg = status.msg + "\n | redis stream doesn't exists";
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        status.isEmpty = false;
        status.code = reqstatus::SERVER_FAILURE;
        status.msg =
            "Carrier server error: redis stream deleted, while on blocking read";
        return false;
    }
    assertReplyType(c2r, reply, REDIS_REPLY_ARRAY);

    // checks the number of streams from the reply
    if (ReadNumStreams(reply) == 0) {
        status.isEmpty = false;
        status.code = reqstatus::SERVER_DOWN;
        status.msg =
            std::string("Carrier server error: no stream found for ") +
            carrierGrp + " carrier group";
        return false;
    }

    // checks if there is no messages from the reply. 
    if (ReadStreamNumMsg(reply, 0) == 0) {
        status.isEmpty = false;
        status.code = reqstatus::SERVER_DOWN;
        status.msg =
            "Carrier server error: expected a message from server, got none";
        return false;
    }
     /*
    int nValsRcvd = ReadStreamMsgNumVal(reply, 0, 0);
    // checks if the elements received is at least MIN_NUM_VALUES
    if (nValsRcvd < minNumVals) {
        status.isEmpty = false;
        status.code = reqstatus::SERVER_ERR;
        status.msg =
            "Carrier server error: number of values must be at least ";
        status.msg += std::to_string(minNumVals);
        return false;
    }

    // checks if the number of values is even
    // must be even, since elements are (field, value) pairs
    if (nValsRcvd % 2 != 0) {
        status.isEmpty = false;
        status.code = reqstatus::SERVER_ERR;
        status.msg = "Carrier server error: number of values must be even; "
                     "consisting of field and value pairs";
        return false;
    }*/

    return true;
}

// checks if the number of values in a reply message
// of type REDIS_REPLY_ARRAY is valid. The number of values is valid if
// they are at least the minNumVals and is even.
bool isValidReplyMsgNumVals(redisContext *c2r, redisReply *reply,
                            reqstatus::RequestStatus &status,
                            unsigned char minNumVals, int ithMsg) {
  assertReply(c2r, reply);
  assertReplyType(c2r, reply, REDIS_REPLY_ARRAY);
  int nValsRcvd = ReadStreamMsgNumVal(reply, 0, ithMsg);
  // checks if the elements received is at least MIN_NUM_VALUES
  if (nValsRcvd < minNumVals) {
    status.isEmpty = false;
    status.code = reqstatus::SERVER_ERR;
    status.msg = "Carrier server error: number of values must be at least ";
    status.msg += std::to_string(minNumVals);
    return false;
  }

  // checks if the number of values is even
  // must be even, since elements are (field, value) pairs
  if (nValsRcvd % 2 != 0) {
    status.isEmpty = false;
    status.code = reqstatus::SERVER_ERR;
    status.msg = "Carrier server error: number of values must be even; "
                 "consisting of field and value pairs";
    return false;
  }
  return true;
}




