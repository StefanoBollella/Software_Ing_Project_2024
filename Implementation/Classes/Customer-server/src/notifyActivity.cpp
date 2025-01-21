#include "main.h"

#define STREAM_IND 0 // index for the request_pong_stream

/*
 * - pong_customer: name of the redis stream for the pong reply to monitor
 * - ping_customer: name of the redis stream for the ping request from monitor
 *
 */
void notifyActivity(redisContext *c2r, const char *pong_customer,
                    const char *ping_customer) {
  const char *SERVER_ID = "server_customer";
  const char *MONITOR_ID = "monitor";
  const char CONSUMER_GROUP_MONITOR[] = "monitor-grp-0";
  redisReply *reply;

  if (streamExists(c2r, ping_customer) && streamExists(c2r, pong_customer)) {
    // reads the ping request from monitor
    reply = (redisReply *)redisCommand(
        c2r, "XREADGROUP GROUP %s customer_server COUNT 2 NOACK STREAMS %s >",
        CONSUMER_GROUP_MONITOR, ping_customer);

    if (reply == nullptr) {
      std::cerr << "Error: impossible to read on " << ping_customer << " stream"
                << std::endl;
      return;
    }

    if (reply->type == REDIS_REPLY_ARRAY && reply->elements > 0) {
      size_t num_msg = ReadStreamNumMsg(reply, STREAM_IND);
      long int num_seq_ping = 0;
      char monitor_id[100];
      char value[100];
      ReadStreamMsgVal(reply, STREAM_IND, num_msg - 1, 0, monitor_id);
      ReadStreamMsgVal(reply, STREAM_IND, num_msg - 1, 1, value);

      if (std::string(monitor_id) == MONITOR_ID) {
        char command[256];
        num_seq_ping = std::stol(value);
        std::cout << "Received PING (seq_n=" << num_seq_ping
                  << ") on stream: " << ping_customer << std::endl;
        dumpReply(reply, 0);

        // PONG reply to the monitor
        snprintf(command, sizeof(command), "XADD %s NOMKSTREAM * %s %ld",
                 pong_customer, SERVER_ID, num_seq_ping);
        redisReply *pong_reply = (redisReply *)redisCommand(c2r, command);

        if (pong_reply == nullptr) {
          std::cerr << "Errore: impossible to send on " << pong_customer
                    << " stream" << std::endl;
        } else {
          std::cout << "Replied PONG (seq_n=" << num_seq_ping
                    << ") to stream: " << pong_customer << std::endl;
        }

        freeReplyObject(pong_reply);
      }
    }

    freeReplyObject(reply);
  }
}
