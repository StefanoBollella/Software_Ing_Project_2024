#include "main.h"

#define MIN_ARG 6 
#define PARSING_ERR -1
#define STREAM_MAX_LEN 5000
#define STREAM_IND 0
#define MAX_BLOCKING 6000 // ms

// #define SEQ_NUM_PONG_START 1000 //Numero di sequenza di partenza

using namespace std;

namespace {

redisContext *c2r;
redisReply *readReply;
redisReply *sendReply;
// redisReply *reply;
char *reqStream;
char *replyStream;
int block;
int pid;
char msgid[100];

//Variabili per la comunicazione con il monitor non funzionale
char *pong_carrier;
char *ping_carrier;
    
//variabile per il parsing dei messaggi 
// char value[100];

// long nanosStart, nanosEnd;
int totMsgs;

const char HOST_NAME[] = "localhost";
const char CARRIER_GROUP[] = "carrier-grp-0";
const unsigned int PORT_NUM = 6379;

Con2DB db("localhost", "5432", "ecommerce_db_usr", "47002", "ecommerce_db");
Con2DB logdb("localhost", "5432", "logdb_usr", "47002", "logdb");

inline void freeRedisStreams(){

    RedisCommand(c2r, "DEL %s", reqStream);
    RedisCommand(c2r, "DEL %s", replyStream);
    /*
    while(streamExists(c2r, reqStream)){
        RedisCommand(c2r, "DEL %s", reqStream);
    }
    while(streamExists(c2r, replyStream)){
        RedisCommand(c2r, "DEL %s", replyStream);
    }
    */
    redisFree(c2r);

}

inline void signalHandlerRemoveStreams(int signal){

    freeRedisStreams();
    cout << "main(): redis streams freed." << endl;
    exit(EXIT_SUCCESS);
}

} // namespace

int main(int argc, char **argv) {

    if(argc < MIN_ARG){
        cerr << "main(): not enought args. requires: exec requestStream replyStream blocking_time" << endl;
        return EXIT_FAILURE;
    }
    
    // setup for freeing streams on closure
    signal(SIGINT, signalHandlerRemoveStreams);
    // signal(SIGKILL, signalHandlerRemoveStreams);

    reqStream = argv[1];
    replyStream = argv[2];
    block = atoi(argv[3]);
    pid = getpid();

    // monitor stuff
    pong_carrier = argv[4];
    ping_carrier = argv[5];

	// ----- connecting to redis ----- //

	cout << "connecting to redis... " << endl;

	c2r = redisConnect(HOST_NAME, PORT_NUM);
	if(c2r == nullptr || c2r->err) {
		cout << "error connecting to redis " << endl;
		exit(EXIT_FAILURE);
	}

	cout << "connected to redis" << endl;

    redisCommand(c2r, "DEL %s", reqStream);
    redisCommand(c2r, "DEL %s", replyStream);

    initStreams(c2r, reqStream, CARRIER_GROUP);
    assert(streamExists(c2r, reqStream));

    initStreams(c2r, replyStream, CARRIER_GROUP);
    assert(streamExists(c2r, replyStream));

    // ------------ 

    char buf[200];
    init_time();
    nanos = get_nanos();

    while(1){

        nanos_day = get_day_nanos(buf);

        // ---------- if the monitor is active and sending "pings", this responds with a "pong" ---------- //
        notifyActivity(c2r, ping_carrier, pong_carrier);

        readReply = RedisCommand(
            c2r, "XREADGROUP GROUP %s server BLOCK %d COUNT 2 NOACK STREAMS %s >", CARRIER_GROUP, block, reqStream
        );

        assertReply(c2r, readReply);
        dumpReply(readReply, 0);

        if(readReply->type == REDIS_REPLY_NIL)    {
            update_time();
            continue;
        }

        assertReplyType(c2r, readReply, REDIS_REPLY_ARRAY);

        totMsgs = ReadStreamNumMsg(readReply, STREAM_IND);
        for(int i = 0; i < totMsgs; ++i){
            
            ReadStreamNumMsgID(readReply, STREAM_IND, i, msgid);

            switch(parse_request(readReply, STREAM_IND, i)) {

                case reqstatus::GET_ORDS_REQ: // get orders here

                    cout << "passing request to getOrdersHandler.. " << endl;
                    getOrdersHandler(c2r, readReply, replyStream, db, i, logdb);

                break;

                case reqstatus::LOSE_ORD_REQ: // lose order here

                    cout << "passing request to loseOrderHandler.. " << endl;
                    loseOrdHandler(c2r, readReply, replyStream, db, i, logdb);

                break;

                case reqstatus::DELIVER_ORD_REQ: // deliver order here

                    cout << "passing request to deliverOrderHandler.. " << endl;
                    deliverOrdHandler(c2r, readReply, replyStream, db, i, logdb);

                break;

                case reqstatus::CARRIER_REG: // register carriers here

                    cout << "passing request to registercarrierHandler.. " << endl;                       
                    registerCarriersHandler(c2r, readReply, replyStream, db, i, logdb);

                break;

                case PARSING_ERR:

                    freeReplyObject(readReply);
                    // freeRedisStreams();
                    redisFree(c2r);
                    return EXIT_FAILURE;

                break;

                default: // none of the above

                    sendReply = RedisCommand(c2r, "XADD %s NOMKSTREAM * type %u code %u msgID %s smg %s", 
                                            replyStream, reqstatus::UNKNOWN, reqstatus::BAD_REQ, msgid, "bad request");

                    assertReplyType(c2r, sendReply, REDIS_REPLY_STRING);
                    freeReplyObject(sendReply);   


            }

        }

        freeReplyObject(readReply);
        update_time();

    } // while(1)

    // freeRedisStreams();
    redisFree(c2r);
    return 0;
}