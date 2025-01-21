#include "main.h"

#define MIN_ARG 6
#define PARSING_ERR -1
#define STREAM_IND 0
#define STREAM_MAX_LEN 50
#define MAX_BLOCKING 6000 // milliseconds

using namespace std;

namespace {

redisContext *c2r;
redisReply *readReply;
redisReply *sendReply;
char *reqStream;
char *replyStream;
char *ping_customer;
char *pong_customer;

int block;
int pid;
char msgid[100];
std::vector<unsigned long> customerIDs;
unsigned long readCounter, sendCounter, iteration;
unsigned reqID;
int totMsgs;

const char HOST_NAME[] = "localhost";
const char CONSUMER_GROUP[] = "cust-grp-0";
const unsigned int PORT_NUM = 6379;

Con2DB db("localhost", "5432", "ecommerce_db_usr", "47002", "ecommerce_db");
Con2DB logdb("localhost", "5432", "logdb_usr", "47002", "logdb");

// is4Init means that the function is called to initiate the program
inline void freeRedisStreams(bool is4Init = false) {
  int attempts = 10;

  while (attempts > 0 &&
         (streamExists(c2r, reqStream) || streamExists(c2r, replyStream))) {
    RedisCommand(c2r, "DEL %s", reqStream);
    RedisCommand(c2r, "DEL %s", replyStream);
    --attempts;
  }
  if (!streamExists(c2r, reqStream)) {
    cout << "main(): pid: " << pid << " freed redis stream " << reqStream
         << "..." << endl;
  }
  if (!streamExists(c2r, replyStream)) {
    cout << "main(): pid: " << pid << " freed redis stream " << replyStream
         << "..." << endl;
  }
  if (!is4Init && c2r != nullptr) {
    redisFree(c2r);
  }
}

inline void initRedisStreams() {
  cout << "main(): pid: " << pid << " connecting to redis ..." << endl;
  // connects to redis server
  c2r = redisConnect(HOST_NAME, PORT_NUM);

  if (c2r == nullptr || c2r->err) {
    if (c2r)
      cerr << "initRedisStreams(): Error: \n" << c2r->errstr << endl;
    else
      cerr << "initRedisStreams(): Can't allocate redis context" << endl;
    exit(EXIT_FAILURE);
  }

  // Removes streams if they exist
  freeRedisStreams(true);

  initStreams(c2r, reqStream, CONSUMER_GROUP);
  assert(streamExists(c2r, reqStream));

  initStreams(c2r, replyStream, CONSUMER_GROUP);
  assert(streamExists(c2r, replyStream));
  cout << "main(): pid: " << pid << " connected to redis ..." << endl;
}

// this is called if CTRL+C are pushed during execution
inline void signalHandlerRemoveStreams(int signal) {
  freeRedisStreams();
  exit(EXIT_SUCCESS);
}

} // namespace

/**
 * Params:
 *  - exec_file: name of the executable file
 *  - reqStream: name of the request redis stream to read from
 *  - replyStream: name of the reply redis stream to write to
 *  - blocking_time: wait time for reading request in milliseconds
 *  - ping_customer: name of the ping stream (monitor's request)
 *  - pong_customer: name of the pong stream (server's reply)
 */
int main(int argc, char **argv) {
  if (argc < MIN_ARG) {
    cerr << "main(): usage: exec_file reqStream replyStream blocking_time "
            "ping_customer pong_customer"
         << endl;
    exit(EXIT_FAILURE);
  }

  // changes the signal handler of SIGINT to signalHandlerRemoveStreams
  signal(SIGINT, signalHandlerRemoveStreams);

  /* Request and Reply streams */
  if (strlen(argv[1]) > STREAM_MAX_LEN) {
    std::cerr << "Error: reqStream exceeds the maximum length of "
              << STREAM_MAX_LEN << " characters." << std::endl;
    return EXIT_FAILURE;
  }
  reqStream = argv[1];

  if (strlen(argv[2]) > STREAM_MAX_LEN) {
    std::cerr << "Error: replyStream exceeds the maximum length of "
              << STREAM_MAX_LEN << " characters." << std::endl;
    return EXIT_FAILURE;
  }
  replyStream = argv[2];

  char *ptr_end;
  errno = 0;
  block = std::strtol(argv[3], &ptr_end, 10);
  if (ptr_end == argv[3]) {
    std::cerr << "Error: invalid format, no digits were found" << std::endl;
    return EXIT_FAILURE;
  }
  if (block > MAX_BLOCKING) {
    std::cerr << "Error: block exceeds " << MAX_BLOCKING << std::endl;
    return EXIT_FAILURE;
  }

  if (strlen(argv[4]) > STREAM_MAX_LEN) {
    std::cerr << "Error: ping_customer exceeds the maximum length of "
              << STREAM_MAX_LEN << " characters." << std::endl;
    return EXIT_FAILURE;
  }
  ping_customer = argv[4];

  if (strlen(argv[5]) > STREAM_MAX_LEN) {
    std::cerr << "Error: pong_customer exceeds the maximum length of "
              << STREAM_MAX_LEN << " characters." << std::endl;
    return EXIT_FAILURE;
  }
  pong_customer = argv[5];

  pid = getpid();
  initRedisStreams();

  cout << "Start main with pid " << pid << endl;

  cout << "Format: " << "time step , " << "global time in second, "
       << "elapsed time in sec, " << "present time in nanosec, "
       << "timestamp, " << "epoch" << endl;

  char timestamp[200];
  init_time();
  nanos = get_nanos();

  while (1) {
    nanos_day = get_day_nanos(timestamp);

    cout << iteration << ", " << global_time_sec << ", " << timeadvance << ", "
         << nanos << ", " << timestamp << ", " << nanos_day << endl;

    // notifies the server activity checker monitor
    // that this customer server is active
    // by responding an eventual PONG to a PING.
    notifyActivity(c2r, pong_customer, ping_customer);

    ++readCounter;
    readReply = RedisCommand(
        c2r, "XREADGROUP GROUP %s server BLOCK %d COUNT 2 NOACK STREAMS %s >",
        CONSUMER_GROUP, block, reqStream);

    printf("Read msg %lu from stream %s\n", readCounter, reqStream);

    assertReply(c2r, readReply);
    dumpReply(readReply, 0);

    if (readReply->type == REDIS_REPLY_NIL) {
      printf("Received no message...\n");
      ++iteration;
      update_time();
      continue;
    }
    assertReplyType(c2r, readReply, REDIS_REPLY_ARRAY);

    totMsgs = ReadStreamNumMsg(readReply, STREAM_IND);
    for (int i = 0; i < totMsgs; ++i) {
      ReadStreamNumMsgID(readReply, STREAM_IND, i, msgid);

      printf("Received a request from customer client...\n");
      printf("Message info: stream: %s, streamnum: %d,"
             " msg: %d, msgid: %s with %d values\n",
             reqStream, STREAM_IND, i, msgid,
             ReadStreamMsgNumVal(readReply, STREAM_IND, i));

      switch (parse_request(readReply, STREAM_IND, i)) {
      case reqstatus::ORDER_REQ:

        printf("Processing order request...\n");
        reqID = orderHandler(c2r, readReply, replyStream, db, i, logdb);

        printf("Sent reply msg %lu to stream %s\n", sendCounter,
               replyStream);
        ++sendCounter;

        break;

      case reqstatus::CANC_ORDER_REQ:
        printf("Processing cancel order request...\n");
        reqID = cancelOrderHandler(c2r, readReply, replyStream, db, i, logdb);

        printf("Sent reply msg %lu to stream %s\n", sendCounter, replyStream);
        ++sendCounter;

        break;

      case reqstatus::PROD_REQ:
        printf("Processing products request...\n");
        getProductsReqHandler(c2r, readReply, replyStream, db, i, logdb);

        printf("Sent reply msg %lu to stream %s\n", sendCounter, replyStream);
        ++sendCounter;
        break;

      case reqstatus::CUST_REGISTER:
        printf("Processing customer registration request...\n");
        registerCustomersHandler(c2r, readReply, replyStream, db, i, logdb);

        printf("Sent reply msg %lu to stream %s\n", sendCounter, replyStream);
        ++sendCounter;

        break;

      case PARSING_ERR:
        printf("Encountered request parsing error :(\n");
        freeReplyObject(readReply);
        freeRedisStreams();
        return EXIT_FAILURE;

      default:
        printf("Received unsupported request...\n");
        sendReply = RedisCommand(
            c2r, "XADD %s NOMKSTREAM * type %u code %u msgID %s msg %s",
            replyStream, reqstatus::UNKNOWN, reqstatus::BAD_REQ, msgid,
            "Bad request, check format");

        assertReplyType(c2r, sendReply, REDIS_REPLY_STRING);
        freeReplyObject(sendReply);

        printf("Sent reply msg %lu to stream %s\n", sendCounter, replyStream);
        ++sendCounter;
      }
    }
    cout << endl;
    freeReplyObject(readReply);
    ++iteration;
    update_time();
  }  // while ()
}
