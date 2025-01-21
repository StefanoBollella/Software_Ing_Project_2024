#include "main.h"
#include "requestReply.h"

#include "../../Carrier-shared/src/utils.h"

#define MIN_NUM_VALUES 6
#define FNAME_LEN 50
#define FVAL_LEN 1000

// c2r, req_stream_main, reply_stream_main, CARRIER_GROUP, status, slice, tempcarIDs
int registerCarrier(
    redisContext *c2r, const char *requestStream, 
    const char *replyStream, const char *carrierGrp, 
    reqstatus::RequestStatus &status, int quantity, std::vector<ulong> &carrIDs
    ){

    if(quantity == 0)
        return EXIT_SUCCESS;
    
    char fname[FNAME_LEN];
    char fval[FVAL_LEN];

    status.type = reqstatus::CARRIER_REG;
    status.isEmpty = false;

    // send the request -----

    redisReply *reply = RedisCommand(
        c2r, "XADD %s NOMKSTREAM * type %u quantity %u",
        requestStream, reqstatus::CARRIER_REG, quantity
    );

    if(!isValidXAddReply(c2r, reply, status)){
        freeReplyObject(reply);
        return EXIT_FAILURE;
    }
    freeReplyObject(reply);

    // get and work with reply -----

    reply = RedisCommand(c2r, "XREADGROUP GROUP %s Alice COUNT 2 BLOCK 5000 NOACK STREAMS %s >", carrierGrp, replyStream);

    if(!isValidXReadGroupReply(c2r, reply, status, carrierGrp)){
        freeReplyObject(reply);
        return EXIT_FAILURE;        
    }

    int numMsg = ReadStreamNumMsg(reply, 0);
    int lstMsg = numMsg - 1;

    dumpReply(reply, 1);

    // analyze the reply ----------

    unsigned char reqType = readReqType(reply, status,lstMsg);
    if(reqType != reqstatus::CARRIER_REG){
        status.isEmpty = false;
        status.type = reqType;
        status.code = reqstatus::SERVER_ERR;
        status.msg = std::string(
            "Carrier server error: got a reply, "
            "but it's request type is different!");
		freeReplyObject(reply);
		return EXIT_FAILURE;        
    }

    const unsigned short MAX_SERVER_STAT_CODE = 399;
    ulong reqCodeRcvd = readReqStatusCode(reply, status,lstMsg);
    if(reqCodeRcvd >= reqstatus::SERVER_ERR && reqCodeRcvd <= MAX_SERVER_STAT_CODE){
        freeReplyObject(reply);
        return EXIT_FAILURE;          
    }

    // check the IDs of those failed to register
    ReadStreamMsgVal(reply, 0, lstMsg, req_formats::RCARREP_REG_IDS_F, fname);
    if (strcmp(fname, "carrIDs")) {
        status.code = reqstatus::SERVER_ERR;
        status.msg = std::string("Malformed response: expected carrIDs, got ") + fname + " instead";
        freeReplyObject(reply);
        return EXIT_FAILURE;
    }
    ReadStreamMsgVal(reply, 0, lstMsg, req_formats::RCARREP_REG_IDS_V, fval);
    carrIDs = strToULVector(fval); // getting the ids 

    memset(fname, '\0', FNAME_LEN);
    memset(fval, '\0', FVAL_LEN);

    // checks the msg of the response
    ReadStreamMsgVal(reply, 0, lstMsg, req_formats::RCARREP_MSG_F, fname);
    if (strcmp(fname, "msg")) {
        status.code = reqstatus::SERVER_ERR;
        status.msg = std::string("Malformed response: expected msg, got ") + fname + " instead";
        freeReplyObject(reply);
        return EXIT_FAILURE;
    }

    ReadStreamMsgVal(reply, 0, lstMsg, req_formats::RCARREP_MSG_V, fval);
    status.msg = std::string(fval);

    freeReplyObject(reply);

    return EXIT_SUCCESS;
}