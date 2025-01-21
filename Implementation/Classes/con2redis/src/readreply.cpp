#include "con2redis.h"
#include "local.h"

/***
 * This file contains functions used to access
 * RESP2/REDIS_REPLY_ARRAY type of redisReply object
 * (see hiredis documentation, in particular XREADGROUP/XREAD). 
 * These functions are useful to access replies from
 * XREADGROUP and XREAD from redis streams.
 *
 * RESP2 Reply
 * One of the following:
 *
 * Array reply: an array where each element is an array composed
 * of a two elements containing the key name and the entries reported
 * for that key. The entries reported are full stream entries, having
 * IDs and the list of all the fields and values. Field and values are
 * guaranteed to be reported in the same order they were added by XADD.
 *
 * Nil reply: if the BLOCK option is given and a timeout occurs, or if
 * there is no stream that can be served.
 *
 */

int ReadNumStreams(redisReply *r)
{
  return (r->elements);  
}  


// reads k-th stream name
int ReadStreamName(redisReply *r, char *streamname,  long unsigned int k)
{
  strcpy(streamname, r->element[k]->element[0]->str);
  
  return(0); 
}  

// reads k-th stream number of messages 
int ReadStreamNumMsg(redisReply *r, long unsigned int streamnum)
{
  
  return(r->element[streamnum]->element[1]->elements);

}  

// k-th stream, i-th msg id
int ReadStreamNumMsgID(redisReply *r, long unsigned int streamnum, int msgnum, char *msgid)
{
  strcpy(msgid, r->element[streamnum]->element[1]->element[msgnum]->element[0]->str);
  return(0);
}


// k-th stream, i-th msg number of values
int ReadStreamMsgNumVal(redisReply *r, long unsigned int streamnum, int msgnum)
{

  return(r->element[streamnum]->element[1]->element[msgnum]->element[1]->elements);

}

// even entry field key, odd entry field value
int ReadStreamMsgVal(redisReply *r, long unsigned int streamnum, int msgnum, int entry, char *value)
{

  strcpy(value, r->element[streamnum]->element[1]->element[msgnum]->element[1]->element[entry]->str);
  return(0);
}

