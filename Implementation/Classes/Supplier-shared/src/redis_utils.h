#ifndef REDIS_UTILS_H
#define REDIS_UTILS_H

#include<iostream>
#include<cstddef>  // for size_t
#include<cstring>  // for strlen
#include<cctype>   // pfor isdigit

#include "handleError.h"
#include "../../con2redis/src/con2redis.h"

//Function to test Redis connection
int connection_test(redisContext *c2r);

//Function to initialize the Redis server and streams
void initialize(redisContext *c2r, HandleError &error,const char *groupName, char* req_stream, char* reply_stream);

//Check if a string represents a positive integer
bool isPositiveInteger(const char* str);

#endif //REDIS_UTILS_H



