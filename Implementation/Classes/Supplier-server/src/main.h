#ifndef main_h
#define main_h

#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h> 
#include<sys/times.h>
#include<cerrno>
#include<cstring>
#include<vector>
#include<unordered_map>
#include<string>
#include <csignal>
#include "serverState.h"
#include "../../Supplier-shared/src/handleReply.h" 
#include "../../Supplier-shared/src/handleRequest.h" 
#include "../../Supplier-shared/src/redis_utils.h" 
#include "../../Clock/src/clock.h" 
#include "../../logger/src/log2db.h" 
#include "../../con2db/pgsql.h" 


REQ_TYPE parseRequest(redisReply* reply, size_t stream_index, size_t message_index, HandleError& error);

void saveProduct(redisContext *c2r, char* reply_stream, redisReply *reply, Con2DB db, Con2DB logdb, 
                 size_t stream_index, size_t message_index, size_t num_val_msg, HandleError& error);


unsigned long saveProductQuery(Con2DB db, const unsigned long id_supplier, 
                               const unsigned int product_quantity, std::string &msg);


void registerSuppliers(redisContext *c2r, char* reply_stream, redisReply *reply, 
                       Con2DB db, size_t stream_index, size_t message_index,
                       size_t num_val_msg, HandleError& error);

std::vector<unsigned long> registerSuppliersQuery(Con2DB db, const int supplier_quantity, std::string &msg);


void infoCurrentProductQuantities(redisContext *c2r, char* reply_stream, 
                                  redisReply *reply ,Con2DB db, size_t stream_index,
                                  size_t message_index, size_t num_val_msg, HandleError& error);

std::vector<unsigned long> infoCurrentProductQuantitiesQuery(Con2DB db, const unsigned long id_supplier, std::string &msg);


void updateProductQuantities(redisContext *c2r, char* reply_stream, redisReply *reply, Con2DB db,
                            Con2DB logdb, size_t stream_index, size_t message_index, size_t num_val_msg, 
                            HandleError& error);
                              
int updateProductQuantitiesQuery(Con2DB db, const unsigned long id_supplier,
                                 std::unordered_map<unsigned long,
                                 unsigned int> products, std::string &msg);


void notifyActivity(redisContext *c2r, const char* req_pong_supplier, const char* reply_ping);

void handleSignal(int signal);


#endif
