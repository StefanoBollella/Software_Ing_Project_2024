#include<random> // useless
#include <iostream>
#include <memory>
#include<vector>
#include "requestReply.h"
#include "redisStreamsUtil.h"

#include "Carrier.h"

#include "../../Carrier-shared/src/RequestStatus.h"
#include "../../Carrier-shared/src/request_formats.h"
#include "../../Carrier-shared/src/utils.h"

#define BUFF_SIZE 500
#define STREAM_IND 0
#define MSG_IND 0

/*
 * i need to send a request to the server of an amount of orders.
 * so the request can be simple with carrierID and number to request (MAX_ORDERS)
 * the reply must contain the status the carrierID and the orderID[] that must be <= MAX_ORDERS
 *
*/
int Carrier::getOrders(redisContext *c2r, const char *reqStream, const char *replyStream, unsigned char MAX_ORDERS, 
						reqstatus::RequestStatus &status, const char* carrierGrp){

	// request with numbers of orders max to take
	if(!isValidRedisStream(c2r, reqStream, status)){
        return EXIT_FAILURE;
    }
    if(!isValidRedisStream(c2r, replyStream, status)){
        return EXIT_FAILURE;
    }

	redisReply *reply;
	// to save the orders use a vector?
	std::vector<unsigned long> orderList;
	char strOrders[BUFF_SIZE];

    // request here -----------------
    reply = RedisCommand(
        c2r, "XADD %s NOMKSTREAM * type %u carr_id %lu order_num %u", 
		reqStream, reqstatus::GET_ORDS_REQ, this->getId(), MAX_ORDERS);

    if (!isValidXAddReply(c2r, reply, status))
        return EXIT_FAILURE;
    freeReplyObject(reply);

	// reply waits 5 secs for -------------
    reply = RedisCommand(c2r, "XREADGROUP GROUP %s Alice COUNT 2 BLOCK 5000 STREAMS %s >",
        carrierGrp, replyStream
    );

	// check reply is good and functioning
    if (!isValidXReadGroupReply( 
        c2r, reply, status, carrierGrp
        )
    )	{
        return EXIT_FAILURE;
	}

    int numMsg = ReadStreamNumMsg(reply, 0);
    int lstMsg = numMsg - 1;

    dumpReply(reply, 1);

    unsigned char reqType = readReqType(reply, status, lstMsg);
    if (reqType != reqstatus::GET_ORDS_REQ){
        status.isEmpty = false;
        status.type = reqType;
        status.code = reqstatus::SERVER_ERR;
        status.msg = std::string(
            "Carrier server error: got a reply, "
            "but it's request type is different!");
		freeReplyObject(reply);
		return EXIT_FAILURE;
	}

    unsigned short MAX_SERVER_STAT_CODE = 399;
    ulong reqCodeRcvd = readReqStatusCode(reply, status,lstMsg);
    // checks if reqCode is a server error
    if (reqCodeRcvd >= reqstatus::SERVER_ERR && reqCodeRcvd <= MAX_SERVER_STAT_CODE){
        return EXIT_FAILURE;
	}

	// if 0 orders were taken return with status no orders avaiable (put it somewhere)

	// make fname & fval, make sure fname (with strcmp) is the one with nOrders, then put in fval the number of orders, if 0 exit.
	char fname[100];
	char fval[100];
    ReadStreamMsgVal(
        reply, STREAM_IND, lstMsg,
        req_formats::GORDREP_N_ORD_F, fname
    );
	if(strcmp(fname, "n_ords")!= 0){
		status.isEmpty = false;
        status.code = reqstatus::SERVER_ERR;
        status.msg =
            std::string(
                "Carrier server error: expected 'n_ords' field, got '") +
                fname + "' instead";
        return EXIT_FAILURE;
	}

    ReadStreamMsgVal(
        reply, STREAM_IND, lstMsg,
        req_formats::GORDREP_N_ORD_V, fval	
    );
    // int prodsRcvd = std::stoi(fval);
	if(std::stoi(fval) == 0){ // there are 0 orders
		return EXIT_SUCCESS;
	}

    ReadStreamMsgVal(
        reply, STREAM_IND, lstMsg,
        req_formats::GORDREP_IDS_V, strOrders // should be correct with this format
    );
	
	// push the orders inside the queue for the carrier
    orderList = strToULVector(strOrders);
	for(unsigned long ordID:orderList){
		
		this->orders.push(ordID);

	}

    freeReplyObject(reply);

    return EXIT_SUCCESS;

}