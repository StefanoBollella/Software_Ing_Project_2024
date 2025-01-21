#ifndef main_h
#define main_h

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <unordered_map>
#include <string>
#include <cassert>

#include "../../../con2db/pgsql.h"
#include "../../../logger/src/log2db.h"
#include "../../../Clock/src/clock.h" 
#include "../../../con2redis/src/con2redis.h" 

void alertMissingServers(const std::unordered_map<std::string, int>& serverStatus);
bool isPositiveInteger(const char* str); 

#endif
