#ifndef main_h
#define main_h

#include "supplier.h"
#include "../../Clock/src/clock.h" //Clock lib
#include "../../Supplier-shared/src/handleReply.h"  //for handling replies from the server
#include "../../Supplier-shared/src/handleRequest.h" //for handling requests to the server
#include "../../Supplier-shared/src/redis_utils.h" //to use the initialize and connection_test functions

#include<iostream>
#include<cerrno>
#include<math.h>
#include<time.h>
#include<stddef.h>
#include<unistd.h> 
#include<sys/types.h>
#include<sys/times.h>
#include<cstdlib> 
#include<algorithm>
#include<queue>
#include<string>
#include<random> //mt19937, uniform_real_distribution, random_device
#include<stdexcept>
#include<limits.h> 
#include<cstring>
#include<sstream> //Include this header for std::ostringstream

#include <chrono> //Include this header for std::chrono::steady_clock and time measurement
#include <thread>  //Include this header for std::this_thread::sleep_for
#include <functional> //Include this header for std::function

//for configuration files 
#include<yaml-cpp/yaml.h>
#include<filesystem>
#include<optional>
#include<fstream>

std::optional<YAML::Node> loadConfigFile(const std::string& configPath) noexcept(false);

STATUS_REQ registerNewProduct(redisContext *c2r,const char* consumer_grp, const int block, const char *req_stream, const char *reply_stream,
                               HandleError& error, const unsigned long id_supplier , unsigned long &id_product, const unsigned int quantities);

STATUS_REQ registerSupplier(redisContext *c2r, const char* consumer_grp, const int block , const char *req_stream, 
                            const char *reply_stream, HandleError& error, int quantity, std::vector<unsigned long> &supplierIDs);
                        
STATUS_REQ  findProductsQuatity(redisContext *c2r, const char* consumer_grp, const int block , const char *req_stream, 
                                 const char *reply_stream, HandleError& error, const unsigned long id_supplier, std::vector<unsigned long> &ProductIDs);            



STATUS_REQ  updateProductQuantities(redisContext *c2r, const char* consumer_grp, const int block , const char *req_stream, 
                                   const char *reply_stream, HandleError& error, const unsigned long id_supplier, std::unordered_map<unsigned long, unsigned int> &productQuantities);            


REPLY_TYPE parseReply(redisReply* reply, size_t stream_index, size_t message_index, HandleError& error);

/** Defines an alias 'CommunicationFunction' for a function type that:
 *   - Takes no parameters.
 *   - Returns a 'STATUS_RE' value.
 *
 * This alias simplifies code by allowing 'CommunicationFunction' to be used 
 * instead of repeatedly writing 'std::function<STATUS_REQ()>'. 
 * 'STATUS_REQ' represents the server response type for this component.
 */
using CommunicationFunction = std::function<STATUS_REQ()>;

STATUS_REQ executeWithTimeout(CommunicationFunction func, int timeout_ms);

bool deleteStreams(redisContext* c2r, const char* reply_stream, const char* req_stream, int max_attempts);

#endif

