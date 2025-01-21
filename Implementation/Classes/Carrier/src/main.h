#ifndef main_h
#define main_h

/*
// constants that will be in the yaml file. temporary.
#define P_LOST 5 // = chance of losing the order; must be 0 <= P_LOST < P_DELIVER.
#define P_DELIVER 45 // = chance of delivering the order; must be P_LOST < P_DELIVER <= 100.
#define P_END_WAIT 66 // = percentage under which the carrier exits waiting status; must be 0 < P_END_WAIT <= 100.
#define WAIT_MOD 2 // = modifier to grow Pmod. the bigger it is the slower Pmod grows; (over 10 already makes it very slow).
#define MAX_ORDERS 10 // = number of max orders a carrier can take;
#define MAX_CARRIERS 1000 // = max carriers permitted in total;

#define Q_GEN_RATIO 10 // = ratio of generation in the while loop.
#define N_INTERVALS 7 // = number of intervals in which objects are generated.
*/

#include<random>
#include<stdlib.h>
#include<time.h>
#include<queue>
#include<cassert>
#include<string>
#include<iostream>
#include<unistd.h>

#include<filesystem>
#include<optional>
#include<stdexcept>
#include<limits.h>
#include <csignal>
#include <yaml-cpp/yaml.h>

#include <sstream> // testing stuff

#include "Carrier.h"

// time
#include "../../Clock/src/clock.h"

// redis
#include "../../Carrier-shared/src/RequestStatus.h"
#include "../../Carrier-shared/src/request_formats.h"
#include "../../Carrier-shared/src/utils.h"

#include "../../con2redis/src/con2redis.h"
#include "../../con2redis/src/local.h"

std::optional<YAML::Node> loadConfigFile(const std::string& configPath) noexcept(false);

int registerCarrier(
    redisContext *c2r, const char *requestStream, 
    const char *replyStream, const char *carrierGrp, 
    reqstatus::RequestStatus &status, int quantity, std::vector<ulong> &carrIDs
    );

#endif // main_h