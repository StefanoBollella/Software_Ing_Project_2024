#include "Carrier.h" 
#include "redisStreamsUtil.h"  
#include "requestReply.h"
#include "../../Carrier-shared/src/RequestStatus.h"
#include "../../Carrier-shared/src/request_formats.h"

#define BUFF_SIZE 100
#define MIN_NUM_VALUES 8

int Carrier::deliverOrder(
	redisContext* c2r, const char *requestStream, const char *replyStream, reqstatus::RequestStatus& status, const char *carrierGrp, unsigned long order_id
)	{

	// is it finished?

	redisReply *reply;
	ulong reqCode, reqResult;
	unsigned char reqType;
	char buff[BUFF_SIZE];
	const unsigned short MAX_SERVER_STAT_CODE = 399;

	// send request -----------------------

	reply = RedisCommand(c2r, "XADD %s NOMKSTREAM * type %u carrierID %lu orderID %lu",
		requestStream, reqstatus::DELIVER_ORD_REQ, this->getId(), order_id
	);

	if(!isValidXAddReply(c2r, reply, status)) {
		freeReplyObject(reply);
		return EXIT_FAILURE;
	}

	// reply -------------------------------

	reply = RedisCommand(c2r, "XREADGROUP GROUP %s Alice COUNT 2 " 
		"BLOCK 5000 NOACK STREAMS %s >",
		carrierGrp, replyStream);

	if (!isValidXReadGroupReply(
		c2r, reply, status, carrierGrp
	)){
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

	// process reply
	reqType = readReqType(reply, status, lstMsg);
	if (reqType != reqstatus::DELIVER_ORD_REQ){
        status.isEmpty = false;
        status.type = reqType;
        status.code = reqstatus::SERVER_ERR;
        status.msg = std::string(
            "Carrier server error: got a reply, "
            "but it's request type is different!");
		freeReplyObject(reply);
		return EXIT_FAILURE;
	}

	// checking if there was a server error
	reqCode = readReqStatusCode(reply, status, lstMsg);
	if (
		reqCode >= reqstatus::SERVER_ERR && reqCode <= MAX_SERVER_STAT_CODE
	){
		freeReplyObject(reply);
		return EXIT_FAILURE;
	}

	memset(buff, '\0', sizeof(buff)); // DREP_CODE_V
	ReadStreamMsgVal(reply,0,lstMsg, req_formats::DREP_CODE_V, buff);
	reqResult = std::stoul(buff);

	if (reqResult == reqstatus::REQ_OK){
		// L'ORDINE È ANDATO A BUON FINE, PUOI PROCEDERE CON IL LAVORO NECESSARIO QUI 
		// cioe l'ordine va tolto dalla coda solo se si arriva a questo punto

		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}