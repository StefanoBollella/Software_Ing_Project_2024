#ifndef MAIN_H
#define MAIN_H


#include <cassert> // for assert
#include <csignal>
#include <iostream>
#include <memory> // for unique_ptr, make_unique
#include <random> // for uniform_int_distribution, random_device, mt19937, random_device
#include <unordered_map>
#include <vector>

#include "Customer.h"
#include "ProductWindow.h"

#include "../../Customer-shared/src/RequestStatus.h"
#include "../../Customer-shared/src/request_formats.h"
#include "../../Customer-shared/src/utils.h"

typedef std::mt19937 rng_type;
typedef std::vector<customer::Customer> customerPool;
typedef customerPool::size_type custPoolSizeT;

int getProducts(
    redisContext *c2r, const char *reqStream,
    const char *replyStream, unsigned char maxNewProducts,
    std::unique_ptr<prodwindow::ProductWindow> &prodWindow,
    reqstatus::RequestStatus &status,
    const char* consumerGrp
) noexcept(false);

ulong generateCustomers(
    custPoolSizeT customerPoolSize, bool* canGrow,
    ulong maxCustomerQty, ulong initCustomerBase,
    rng_type randGen, ulong* customerGrowthBase
);

int registerCustomers(
    redisContext *c2r, const char *requestStream,
    const char *replyStream, const char* consumerGrp,
    reqstatus::RequestStatus &status, unsigned cust_qty,
    std::vector<ulong> &regIDs
);

#include <filesystem> // must be C++17 or above
#include <limits.h>   // PATH_MAX and NAME_MAX
#include <optional>   // must be C++17 or above
#include <stdexcept>  // invalid_argument exception
#include <yaml-cpp/yaml.h>

std::optional<YAML::Node>
loadConfigFile(const std::string& configPath) noexcept(false);

// REDIS LIBS
#include "../../con2redis/src/con2redis.h"
#include "../../con2redis/src/local.h"

// Clock
#include "../../Clock/src/clock.h"

#endif // MAIN_H
