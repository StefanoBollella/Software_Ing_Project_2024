#include "main.h"

#define MIN 0
#define MAX 1
#define MIN_ARG 6
#define STREAM_MAX_LEN 50
#define MAX_BLOCKING 6000 //milleseconds

using namespace std;

namespace {

double INITIAL_WAITING_THRESHOLD;
double RANGE_PROB;
double REDUCTION_FACTOR;
unsigned int MAX_ID_PRODUCTS;
unsigned int MAX_SUPPLIER_QUEUE;
unsigned int Q_GEN_RATIO;
unsigned int N_INTERVALS;
unsigned int MIN_PRODUCT_Q;
unsigned int MAX_PRODUCT_Q;

redisContext *c2r;
char *req_stream;
char *reply_stream;
int block;
int pid;

const char HOST_NAME[] = "localhost";
const unsigned int PORT_NUM = 6379;
const char CONSUMER_GROUP[] = "supplier-grp-0";

const int max_attempts = 10; //Maximum attempts to delete streams
const int TIMEOUT_MS = 6000; //Timeout in milleseconds for all operations 

/**
 * Establishes a connection to the Redis server and performs a connection test.
 * If the connection fails or any error occurs during initialization, the error is handled
 * and the client is terminated to prevent further execution.
 */

inline void initRedisStreams(HandleError &error, int pid) {
  try {
    //Trying to connect to redis
    printf("main(): pid %d: connecting to redis... \n", pid);
    c2r = redisConnect(HOST_NAME, PORT_NUM);

    if (c2r == nullptr || c2r->err) {
      error.handleError(ERR_CONNECTION_FAILED,
                        "Redis connection error.\n"); 
                                                     
    }
    printf("main(): pid %d: connected to redis\n", pid);

    if (connection_test(c2r) != 0) {
      error.handleError(ERR_CONNECTION_TEST_FAILED,
                        "Error while checking connection to Redis.");
    }

    printf("main(): pid %d: Client is ready.\n", pid);
  } catch (const std::runtime_error &e) {
    cerr << "Critical error during server startup: " << e.what() << endl;

    if (c2r != nullptr) {
      redisFree(c2r);
    }
    cout << "Client terminated because of critical error." << endl;
    exit(EXIT_FAILURE);
  }
}
/**
 * This function extracts and assigns values for supplier configuration parameters,
 * verifying their validity through assertions to ensure logical constraints are met
 * (e.g., thresholds, ranges, and limits for iterations and product quantities).
 */
inline void loadAllConfigParams(optional<YAML::Node> &configParams) {

  INITIAL_WAITING_THRESHOLD =
      (*configParams)["INITIAL_WAITING_THRESHOLD"].as<double>();
  RANGE_PROB = (*configParams)["RANGE_PROB"].as<double>();
  REDUCTION_FACTOR = (*configParams)["REDUCTION_FACTOR"].as<double>();
  MAX_ID_PRODUCTS = (*configParams)["MAX_ID_PRODUCTS"].as<unsigned int>();
  MAX_SUPPLIER_QUEUE = (*configParams)["MAX_SUPPLIER_QUEUE"].as<unsigned int>();
  Q_GEN_RATIO = (*configParams)["Q_GEN_RATIO"].as<unsigned int>();
  N_INTERVALS = (*configParams)["N_INTERVALS"].as<unsigned int>();
  MIN_PRODUCT_Q = (*configParams)["MIN_PRODUCT_Q"].as<unsigned int>();
  MAX_PRODUCT_Q = (*configParams)["MAX_PRODUCT_Q"].as<unsigned int>();

  assert(INITIAL_WAITING_THRESHOLD > MIN && INITIAL_WAITING_THRESHOLD < MAX);
  assert(RANGE_PROB > MIN && (INITIAL_WAITING_THRESHOLD + RANGE_PROB) < MAX);
  assert(REDUCTION_FACTOR > MIN &&
         REDUCTION_FACTOR < INITIAL_WAITING_THRESHOLD);
  assert(N_INTERVALS > MIN && N_INTERVALS <= MAX_SUPPLIER_QUEUE);
  assert(MIN_PRODUCT_Q < MAX_PRODUCT_Q);

  const unsigned int MAX_UINT = std::numeric_limits<unsigned int>::max();

  /* Checking for a possible overflow in the maximum number of iterations
   * allowed for a supplier in the “waiting” state
   */
  const double potential_iterations =
      INITIAL_WAITING_THRESHOLD / REDUCTION_FACTOR;
  assert(potential_iterations <= MAX_UINT &&
         "Potential number of iterations exceeds int limit");

  assert(MAX_ID_PRODUCTS > MIN && MAX_ID_PRODUCTS <= MAX_UINT);
  assert(MAX_SUPPLIER_QUEUE <= MAX_UINT);
  assert(Q_GEN_RATIO <= MAX_UINT);
  assert(N_INTERVALS <= MAX_UINT);
  assert(MIN_PRODUCT_Q <= MAX_UINT);
  assert(MAX_PRODUCT_Q <= MAX_UINT);
}

} // namespace
/**
 * Attempts to delete `reply_stream` and `req_stream` repeatedly until both are successfully removed
 * or the maximum number of attempts is reached. Returns true if both streams are deleted.
 */
bool deleteStreams(redisContext* c2r, const char* reply_stream, 
                   const char* req_stream, int max_attempts) {
    
    redisReply* delReply;
    int attempts = 0;
    bool reply_stream_deleted = false;
    bool req_stream_deleted = false;

    while(attempts < max_attempts && (!reply_stream_deleted || !req_stream_deleted)) {
        if (!reply_stream_deleted) {
            delReply = (redisReply*)redisCommand(c2r, "DEL %s", reply_stream);
            if (delReply != nullptr) {
                if (!streamExists(c2r, reply_stream)) {
                    reply_stream_deleted = true;
                    std::cout << "reply_stream eliminato con successo." << std::endl;
                }
                freeReplyObject(delReply);
            }
        }
        if (!req_stream_deleted) {
            delReply = (redisReply*)redisCommand(c2r, "DEL %s", req_stream);
            if (delReply != nullptr) {
                if (!streamExists(c2r, req_stream)) {
                    req_stream_deleted = true;
                    std::cout << "req_stream eliminato con successo." << std::endl;
                }
                freeReplyObject(delReply);
            }
        }
        attempts++;
    }
    return reply_stream_deleted && req_stream_deleted;
}


int main(int argc, char **argv) {

  assert(argc < MIN_ARG);

  optional<YAML::Node> configParams;
  try {
    //load parameters from config. file
    configParams = loadConfigFile(argv[1]);
  } catch (const invalid_argument &e) {
    cerr << "Exception caught:" << e.what() << endl;
  }

  if (!configParams.has_value())
    return EXIT_FAILURE;

  loadAllConfigParams(configParams);

  if (strlen(argv[2]) > STREAM_MAX_LEN) {
    std::cerr << "Error: req_stream exceeds the maximum length of "
              << STREAM_MAX_LEN << " characters." << std::endl;
    return EXIT_FAILURE;
  }
  req_stream = argv[2];

  if (strlen(argv[3]) > STREAM_MAX_LEN) {
    std::cerr << "Error: reply_stream exceeds the maximum length of "
              << STREAM_MAX_LEN << " characters." << std::endl;
    return EXIT_FAILURE;
  }
  reply_stream = argv[3];

  block = atoi(argv[4]);
  if (block > MAX_BLOCKING) {
    std::cerr << "Error: block exceeds " << MAX_BLOCKING << std::endl;
    return EXIT_FAILURE;
  }

  cout << "main(): Client args passed:" << endl;
  cout << " - config_file:  " << argv[1] << endl;
  cout << " - req_stream: " << argv[2] << endl;
  cout << " - reply_stream: " << argv[3] << endl;
  cout << " - block: " << argv[4] << endl;

  pid = getpid();
  HandleError error;
  initRedisStreams(error, pid); //Connection to Redis

  char buf[200]; //clock

  random_device rd;
  mt19937 gen(rd());
  uniform_real_distribution<double> uDist(0.0, 1.0);
  uniform_int_distribution<unsigned int> uDistUnsigned(MIN_PRODUCT_Q,
                                                       MAX_PRODUCT_Q);
  double random_state;
  unsigned int random_quantities;

  int slice = MAX_SUPPLIER_QUEUE / (double)N_INTERVALS;
  bool full_gen = false;
  queue<Supplier> supplierQueue;
  double current_wt; //current waiting threshold

  init_time();
  nanos = get_nanos();
  int iteration = -1;

  printf("%-10s %-15s %-15s %-15s %-15s\t %-20s %-15s %-15s %-15s\n",
         "Iteration", "ID Supplier", "Global Time", "Timeadvance", "Nanos",
         "Buffer", "Nanos Day", "Current State", "Activity");

  while (true) {
    iteration++;

    nanos_day = get_day_nanos(buf);

    if (streamExists(c2r, req_stream) && streamExists(c2r, reply_stream)) {

      if ((iteration % Q_GEN_RATIO == 0) && !full_gen && slice > 0) {

        if (static_cast<int>(supplierQueue.size()) + slice >
            static_cast<int>(MAX_SUPPLIER_QUEUE)) {

          slice = MAX_SUPPLIER_QUEUE - supplierQueue.size();
          full_gen = true;
        }
        if (supplierQueue.size() < MAX_SUPPLIER_QUEUE) {
          try {
              vector<unsigned long> tempsupIDs;
            
              //Lambda function definition for registerSupplier
              auto registerSupplierFunc = [&]() -> STATUS_REQ {
                   return registerSupplier(c2r, CONSUMER_GROUP,
                              block, req_stream,
                              reply_stream, 
                              error, slice,
                               tempsupIDs);
              };
              STATUS_REQ status_req = executeWithTimeout(registerSupplierFunc, TIMEOUT_MS);
             
             //std::cout<<" SATUS REQ "<<status_req<<" REGISTER NEW SUPPLIER"<<std::endl;
             
             if (status_req == REQ_SUCCESS) {
                for (unsigned long id : tempsupIDs) {
                 

                  Supplier newSupplier(id, INITIAL_WAITING_THRESHOLD,
                                       REDUCTION_FACTOR, MAX_ID_PRODUCTS);
                  //std::cout << "Copy completed new id supplier "<<id<< std::endl;
                  supplierQueue.push(newSupplier);
                }
                tempsupIDs.clear();
                
              } 
              else if (status_req == REQ_FAILED) {
                error.handleError(ERR_PROCESSING_FAILED,
                                  "Registration of new suppliers failed.");
              }
              else if (status_req == BAD_REQUEST) {
                error.handleError(
                    ERR_STREAM_PARSE_FAILED,
                    "Request sent to the server with an incorrect format.");
              }
              else if (status_req == TIMEOUT_FAILED){
                 cerr<< "Timeout scaduto o registrazione fallita"<< endl;
                //Deletes streams and moves on to the next iteration
                  bool streamsDeleted = deleteStreams(c2r, reply_stream, req_stream, max_attempts);
                  if (!streamsDeleted) {
                        std::cerr << "Errore durante l'eliminazione degli stream. Tentativo fallito." << std::endl;
                  }
                 micro_sleep(400000);
                 update_time();
                 continue; 
             }
          
          }
          catch (const std::runtime_error &e) {
                cerr << " " << e.what() << endl;
                cout << "Registration of new " << slice << " suppliers failed." << endl;
          }
        }
      }

      //If the queue is empty, then it is necessary to move on to the next iteration
      if(supplierQueue.size() == 0){
            printf("%-10d %-15d %-15.5lf %-15.5lf %-20ld %-20s %-15ld %-15d Supplier queue size %ld\n",
                     iteration, 0, global_time_sec, timeadvance,
                     nanos, buf, nanos_day, -1, supplierQueue.size());
            micro_sleep(400000); //Sleep per 0,4s  
            update_time(); 
            continue; 
      }  

      Supplier currentSupplier = supplierQueue.front();
      state current_state = currentSupplier.get_current_state();

      printf("%-10d %-15lu %-15.5lf %-15.5lf %-20ld %-20s %-15ld %-15d",
             iteration, currentSupplier.get_id(), global_time_sec, timeadvance,
             nanos, buf, nanos_day, current_state);

      switch (current_state) {

      case starting:
        try {
          currentSupplier.set_current_state(generate_product);
        } catch (const std::invalid_argument &e) {
          cerr << "Error setting state: " << e.what() << endl;
        }
        break;

      case waiting:
        try {
          current_wt = currentSupplier.update_waiting_threshold();
        } catch (const std::runtime_error &e) {
          cerr << "Error updating wait threshold: " << e.what() << endl;
        }
        /* Random choice of state for the next iteration */
        random_state = uDist(gen); 

        if (currentSupplier.get_num_generated_products() >= MAX_ID_PRODUCTS) {

          if (random_state >= 0 && random_state <= current_wt) {
            currentSupplier.increase_waiting_iterations();
            break;
          } else if (random_state > current_wt && random_state <= 1) {

            try {
              currentSupplier.set_current_state(update);
            } catch (const std::invalid_argument &e) {
              cerr << "Error setting state: " << e.what() << endl;
            }
            break;
          }
        } else {
          if (random_state >= 0 && random_state <= current_wt) {
            currentSupplier.increase_waiting_iterations();
            break;
          } else if (random_state > current_wt &&
                     random_state <= (current_wt + RANGE_PROB)) {

            try {
              currentSupplier.set_current_state(generate_product);
            } catch (const std::invalid_argument &e) {
              cerr << "Error setting state: " << e.what() << endl;
            }
            break;
          } else if (random_state > (current_wt + RANGE_PROB) &&
                     random_state <= 1) {

            try {
              currentSupplier.set_current_state(update);
            } catch (const std::invalid_argument &e) {
              cerr << "Error setting state: " << e.what() << endl;
            }
            break;
          }
          break;
        }
        break;
      case generate_product:
        try {
          unsigned long id_product;
          unsigned long id_supplier = currentSupplier.get_id();
          random_quantities = uDistUnsigned(gen);

          
           //Lambda function definition
           auto registerNewProductFunc = [&]() -> STATUS_REQ {
                   return registerNewProduct(c2r, CONSUMER_GROUP,
                                             block, req_stream, 
                                             reply_stream, error,
                                             id_supplier, id_product,
                                             random_quantities);
              };
              STATUS_REQ status_req = executeWithTimeout(registerNewProductFunc, TIMEOUT_MS);
          
          // std::cout<<" SATUS REQ "<<status_req<<" REGISTER NEW PRODUCT "<<std::endl;
          if (status_req == REQ_SUCCESS) {
            currentSupplier.new_product(id_product, random_quantities);
            printf(" New Product id: %lu q: %u", id_product,
                   random_quantities); // log
          } 
          else if (status_req == REQ_FAILED) {
                     error.handleError(ERR_PROCESSING_FAILED,
                              "New product registration failed.");
           }
           else if (status_req == BAD_REQUEST) {
                     error.handleError(
                           ERR_STREAM_PARSE_FAILED,
                           "Request sent to the server with an incorrect format.");
           }
           else if (status_req == TIMEOUT_FAILED){
                      cerr<< "Timeout scaduto o registrazione fallita"<< endl;
                      //Deletes streams and moves on to the next iteration
                      bool streamsDeleted = deleteStreams(c2r, reply_stream, req_stream, max_attempts);
                      if (!streamsDeleted) {
                        std::cerr << "Errore durante l'eliminazione degli stream. Tentativo fallito." << std::endl;
                      }
                       micro_sleep(400000);
                       update_time();
                       continue; 
          }
        } catch (const std::out_of_range &e) {
          cerr << "Out of range erorr: " << e.what() << endl;
        } catch (const std::logic_error &e) {
          cerr << "Error : " << e.what() << endl;
        } catch (const std::runtime_error &e) {
          cerr << " " << e.what() << endl;
          cout << "The new product could not be register." << endl;
        }
        try {
          currentSupplier.set_current_state(waiting);
        } catch (const std::invalid_argument &e) {
          cerr << "Error setting state: " << e.what() << endl;
        }
        break;

      case update:

        try {
        
             vector<unsigned long> tempProductIDs;
             unsigned long id_supplier = currentSupplier.get_id();

             //Search for product IDs that have the same minimum current quantity
             //among the available products by id_supplier.

             //Lambda function definition for findProductsQuatity
             auto findProductsQuatityFunc = [&]() -> STATUS_REQ {
                   return findProductsQuatity( c2r, CONSUMER_GROUP, 
                                                block, req_stream,
                                                reply_stream, error,
                                                id_supplier, tempProductIDs);
             };
             STATUS_REQ status_req = executeWithTimeout(findProductsQuatityFunc, TIMEOUT_MS);
            // std::cout<<" SATUS REQ "<<status_req<<" findProductQuanity "<<std::endl;
             if(status_req == REQ_SUCCESS) {

                std::unordered_map<unsigned long, unsigned int> productQuantities;
                printf(" Update ");
            
                for (unsigned long id : tempProductIDs) {
                        unsigned int random_quantity = uDistUnsigned(gen);
                        productQuantities[id] = random_quantity;
                        printf(" ID :%lu Q: %u", id, random_quantity);
                }
                
                //Lambda function definition for updateProductQuantities
                auto updateProductQuantitiesFunc = [&]() -> STATUS_REQ {
                        return updateProductQuantities(c2r, CONSUMER_GROUP,
                                                       block, req_stream, 
                                                       reply_stream, error,
                                                       id_supplier, 
                                                       productQuantities);

                };
                STATUS_REQ status_req = executeWithTimeout(updateProductQuantitiesFunc, TIMEOUT_MS);
               
                if(status_req == REQ_SUCCESS) {
                   //Updating quantities in the Supplier only in case of success
                   for (const auto& [id, quantity] : productQuantities) {
                       currentSupplier.add_quantity(id, quantity);
                   }                
                }
                else if (status_req == REQ_FAILED) {
                     error.handleError(ERR_PROCESSING_FAILED,
                                "Failed to update product quantities.");
                } 
                else if (status_req == BAD_REQUEST) {
                          error.handleError(
                          ERR_STREAM_PARSE_FAILED,
                          "Request sent to the server with an incorrect format.");
                }
                else if (status_req == TIMEOUT_FAILED){
                      cerr<< "Timeout scaduto o registrazione fallita"<< endl;
                      //Deletes streams and moves on to the next iteration
                      bool streamsDeleted = deleteStreams(c2r, reply_stream, req_stream, max_attempts);
                      if (!streamsDeleted) {
                        std::cerr << "Errore durante l'eliminazione degli stream. Tentativo fallito." << std::endl;
                      }
                      micro_sleep(400000);
                      update_time();
                     continue; 
              }        
          }
          else if (status_req == REQ_FAILED) {
                     error.handleError(ERR_PROCESSING_FAILED,
                                "Failed to update product quantities.");
          } 
          else if (status_req == BAD_REQUEST) {
                          error.handleError(
                           ERR_STREAM_PARSE_FAILED,
                           "Request sent to the server with an incorrect format.");
           }
           else if (status_req == TIMEOUT_FAILED){
                      cerr<< "Timeout scaduto o registrazione fallita"<< endl;
                         //Deletes streams and moves on to the next iteration
                      bool streamsDeleted = deleteStreams(c2r, reply_stream, req_stream, max_attempts);
                      if (!streamsDeleted) {
                        std::cerr << "Errore durante l'eliminazione degli stream. Tentativo fallito." << std::endl;
                      }
                      micro_sleep(400000);
                      update_time();
                     continue; 
           } 
          
        }//try
        catch (const std::runtime_error &e) {
          cerr << " " << e.what() << endl;
          cout << "Unable to update products." << endl;
        }

        try {
          currentSupplier.set_current_state(waiting);
        } catch (const std::invalid_argument &e) {
          cerr << "Error setting state: " << e.what() << endl;
        }
        break;

      default:
        break;
      } // switch

      supplierQueue.pop();
      Supplier newSupplier(currentSupplier);
      supplierQueue.push(newSupplier);
      printf("\n");

    } // if
    else {
      printf("%-10d %-15d %-15.5lf %-15.5lf %-20ld %-20s %-15ld %-15d %-15s\n",
             iteration, 0, global_time_sec, timeadvance, nanos, buf, nanos_day,
             -1, "The supplier server is temporarily down.");
    }

    micro_sleep(400000); // Sleep per 0,4s
    update_time();
  }
  redisFree(c2r);
  return 0;
}
