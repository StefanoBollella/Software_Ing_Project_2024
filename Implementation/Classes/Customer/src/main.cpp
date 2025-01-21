#include "main.h"
#include "Customer.h"
#include "ProductWindow.h"
#include <hiredis/hiredis.h>
#include <iostream>

using namespace std;

// contains this file's constants and helper functions
namespace {

custPoolSizeT customerPoolSize;

ulong totalOrders;          // total order attempts
ulong totalCancelledOrders; // total cancellation attempts
ulong iteration;
ulong totalCustomers;
ulong custQty2Generate;

redisContext *c2r;

ulong orderLimit;
ulong maxCustQty;
ulong cancelProb;
ulong logoutProb;
ulong shoppingProb;
ulong maxProducts;
ulong restockQty;
ulong cycleCustGenRatio;
ulong initCustBase;
ulong cycleProdGenRatio;
int pid;
char *yamlConf;
ulong totProdQty;
ulong prodQty;

// --------------------------------------------------------------
/* Following constants can be migrated to the config file */
// REDIS CONSTANTS (C++ constants)
const char HOST_NAME[] = "localhost";
const char CONSUMER_GROUP[] = "cust-grp-0";

char *req_stream;
char *reply_stream;

const uint PORT_NUM = 6379;
const ulong MAX_PROB = 100;
const char MIN_ARG = 4;
const ulong SLEEP_MS = 500000;
// --------------------------------------------------------------

inline void initRedisStreams() {
  cout << "main(): pid: " << pid << " connecting to redis ..." << endl;
  c2r = redisConnect(HOST_NAME, PORT_NUM);

  if (c2r == nullptr || c2r->err) {
    if (c2r)
      cerr << "initRedisStreams(): Error: \n" << c2r->errstr << endl;
    else
      cerr << "initRedisStreams(): Can't allocate redis context" << endl;
    exit(EXIT_FAILURE);
  }
  cout << "main(): pid: " << pid << " connected to redis ..." << endl;
}

inline void freeRedisStreams() {
  int attempts = 10;

  while (attempts > 0 &&
         (streamExists(c2r, req_stream) || streamExists(c2r, reply_stream))) {
    RedisCommand(c2r, "DEL %s", req_stream);
    RedisCommand(c2r, "DEL %s", reply_stream);
    --attempts;
  }

  if (!streamExists(c2r, req_stream)) {
    cout << "main(): pid: " << pid << " freed redis stream " << req_stream
         << "..." << endl;
  }

  if (!streamExists(c2r, reply_stream)) {
    cout << "main(): pid: " << pid << " freed redis stream " << reply_stream
         << "..." << endl;
  }

  // we don't clean the redisContext (c2r) since
  // we only want to delete the redis streams
}

// this is called if CTRL+C are pushed during execution
inline void signalHandlerPrintInfo(int signal) {
  cout << "Received SIGINT, cleaning up Redis context..." << endl;

  if (c2r != nullptr) {
    redisFree(c2r);
  }

  cout << "\nTerminated by signal SIGINT" << endl;
  cout << "Total customers generated: " << totalCustomers << endl;
  cout << "Total customers remaining: " << customerPoolSize << endl;
  cout << "Total orders: " << totalOrders << endl;
  cout << "Total cancelled orders: " << totalCancelledOrders << endl;
  cout << "Total iterations: " << iteration << endl;

  exit(EXIT_SUCCESS);
}

inline std::string throwMsg(std::string func, std::string msg) {
  return std::string(__FILE__) + ":" + func + "():" + std::to_string(__LINE__) +
         ": " + msg;
}

inline void loadAllConfigParams(optional<YAML::Node> &configParams) {
  try {
    orderLimit = configParams.value()["ORDER_LIMIT"].as<ulong>();

    maxCustQty = configParams.value()["MAX_CUST_QTY"].as<ulong>();

    cancelProb = configParams.value()["CANCEL_PROB"].as<ulong>();

    logoutProb = configParams.value()["LOGOUT_PROB"].as<ulong>();

    shoppingProb = configParams.value()["SHOPPING_PROB"].as<ulong>();

    maxProducts = configParams.value()["MAX_PRODUCTS"].as<ulong>();

    restockQty = configParams.value()["RESTOCK_QTY"].as<ulong>();

    cycleCustGenRatio =
        configParams.value()["CYCLE_CUST_GEN_RATIO"].as<ulong>();

    initCustBase = configParams.value()["INIT_CUST_BASE"].as<ulong>();

    cycleProdGenRatio =
        configParams.value()["CYCLE_PROD_GEN_RATIO"].as<ulong>();
  } catch (const YAML::TypedBadConversion<ulong> &e) {
    cerr << "loadAllConfigParams(): Err: " << e.what()
         << " ..."
            "\n  | Check if param names in loadAllConfigParams()"
         << " and in the yaml config file correspond ... "
         << "\n  | Check if the param values have the right sign"
         << " and types ... " << endl;
    redisFree(c2r);
    exit(EXIT_FAILURE);
  }

  if (initCustBase == 0 || initCustBase > maxCustQty)
    throw domain_error(
        throwMsg(__func__, "0 < initCustBase <= maxCustQty must be true"));

  if (cancelProb >= logoutProb)
    throw domain_error(
        throwMsg(__func__, "cancelProb < logoutProb must be true"));

  if (logoutProb >= shoppingProb)
    throw domain_error(
        throwMsg(__func__, "logoutProb < shoppingProb must be true"));

  if (shoppingProb >= MAX_PROB)
    throw domain_error(throwMsg(__func__, "shoppingProb < 100 must be true"));

  if (maxProducts == 0 || maxProducts > 100)
    throw domain_error(
        throwMsg(__func__, "0 < maxProducts < 100 must be true"));

  if (restockQty > 15)
    throw domain_error(throwMsg(__func__, "0 <= restockQty < 16 must be true"));

  if (cycleCustGenRatio < 100)
    throw domain_error(
        throwMsg(__func__, "cycleCustGenRatio >= 100 must be true"));

  if (cycleProdGenRatio < 100)
    throw domain_error(
        throwMsg(__func__, "cycleProdGenRatio >= 100 must be true"));

  // if zero then set to the max possible value
  if (maxCustQty == 0)
    maxCustQty = ULONG_MAX;
}

inline void nextIter() {
  ++iteration;
  micro_sleep(SLEEP_MS);
  // updates nanos, global_time_sec, timeadvance
  update_time();
}

inline std::string getStateName(int state) {
  switch (state) {
  case customer::IDLE:
    return "IDLE";
  case customer::SHOPPING:
    return "SHOPPING";
  case customer::CANCELLING:
    return "CANCELLING";
  case customer::TERMINATED:
    return "TERMINATED";
  default:
    return "UNKNOWN";
  }
}

} // namespace

/**
 * params:
 *  - exec_name: the name of the executable
 *  - yaml_conf: yaml config file containing extra parameters
 *  - req_stream: name of the request redis stream of main server
 *  - reply_stream: name of the reply redis stream of main server
 */
int main(int argc, char **argv) {
  if (argc < MIN_ARG) {
    cerr << "main(): usage: exec_name yaml_conf "
            "req_stream reply_stream "
         << endl;
    cerr << "Check the number of arguments..." << endl;
    exit(EXIT_FAILURE);
  }

  yamlConf = argv[1];
  req_stream = argv[2];
  reply_stream = argv[3];

  optional<YAML::Node> configParams(loadConfigFile(argv[1]));
  if (!configParams.has_value()) {
    cerr << "main(): failed to load parameters from " << argv[1] << endl;
    exit(EXIT_FAILURE);
  }

  loadAllConfigParams(configParams);

  // changes the signal handler of SIGINT to signalHandlerPrintInfo
  signal(SIGINT, signalHandlerPrintInfo);

  pid = getpid();
  // ----------------------- Probability Vars ---------------------------
  rng_type::result_type currentProb;
  random_device rand_seed; // seed generator
  uniform_int_distribution<rng_type::result_type> uDist(0, 10e6 - 1);
  ulong seed = rand_seed();
  rng_type randGen(seed);

  cout << "Start main with pid " << pid << " ppid " << getppid() << " seed "
       << seed << endl;
  cout << "Format: " << "time step, " << "global time in second, "
       << "elapsed time in sec, " << "present time in nanosec, "
       << "customerID, " << "state, " << "timestamp, " << "epoch" << endl;

  initRedisStreams();

  // ---------------------- Customers Initialization --------------------
  auto customerPoolPtr = make_unique<vector<customer::Customer>>();
  customer::Customer *currentCustomer;
  uint customerIndex;
  ulong customerGrowthBase = 0;
  bool canGrow = true;

  // ---------------------- Product Window Initialization ----------------
  auto prodWindowPtr = make_unique<prodwindow::ProductWindow>(0);
  auto orderPtr = make_unique<unordered_map<ulong, ulong>>();
  reqstatus::RequestStatus status = reqstatus::RequestStatus{};
  // ----------------------------------------------------------------------
  ulong prodID, numProductsToOrder, numProducts;
  int orderRes, getProdsRes;
  char timestamp[200];
  init_time();
  nanos = get_nanos();
  vector<ulong> regIDs;

  // PROBABILITY INTERVAL
  // 0  <= cancelProb < logoutProb < shoppingProb <= IDLE_PROB = 100
  do {
    // nanos after second of the timestamp
    nanos_day = get_day_nanos(timestamp);

    if (!streamExists(c2r, req_stream) || !streamExists(c2r, reply_stream)) {
      cout << "Waiting for streams from server..." << endl;
      nextIter();
      continue;
    }

    assert(cycleCustGenRatio != 0);
    if (iteration % cycleCustGenRatio == 0 || customerPoolSize == 0) {
      // ----------------------------------------------
      // CUSTOMER GENERATION
      custQty2Generate =
          generateCustomers(customerPoolSize, &canGrow, maxCustQty,
                            initCustBase, randGen, &customerGrowthBase);

      // ----------------------------------------------
      // CUSTOMER REGISTRATION
      status = reqstatus::RequestStatus{};
      regIDs.clear();
      if (custQty2Generate > 0) {
        if (registerCustomers(c2r, req_stream, reply_stream, CONSUMER_GROUP,
                              status, custQty2Generate,
                              regIDs) == EXIT_SUCCESS) {
          for (ulong id : regIDs)
            customerPoolPtr->emplace_back(customer::Customer(id, orderLimit));
          totalCustomers += custQty2Generate;
          customerPoolSize = customerPoolPtr->size();

          status = reqstatus::RequestStatus{};
        } else {
          cerr << "Customer registration failed: code: " << status.code << " "
               << status.msg << endl;
          if (status.code == reqstatus::SERVER_DOWN) {
            cerr << "Server response timeout, "
                    "server maybe down..."
                 << endl;
            freeRedisStreams();
          }
        }
      }
      // ----------------------------------------------
    }
    assert(cycleProdGenRatio > 0);
    status = reqstatus::RequestStatus{};
    if (iteration % cycleProdGenRatio == 0) {
      getProdsRes = getProducts(c2r, req_stream, reply_stream, restockQty,
                                prodWindowPtr, status, CONSUMER_GROUP);
      if (getProdsRes == EXIT_FAILURE) {
        cerr << __func__ << "(): getProducts: " << status.msg << endl;
        if (status.code == reqstatus::SERVER_DOWN) {
          cerr << "Server response timeout, "
                  "server maybe down..."
               << endl;
          freeRedisStreams();
        }
        nextIter();
        continue;
      }
      status = reqstatus::RequestStatus{};
    }

    if (customerPoolSize == 0) {
      cout << "Customer pool is empty..." << endl;
      nextIter();
      continue;
    }

    /* CUSTOMER RANDOM CHOOSE */
    assert(customerPoolSize > 0);
    customerIndex = uDist(randGen) % customerPoolSize;
    assert(customerIndex >= 0 && customerIndex < customerPoolSize);
    currentCustomer = &customerPoolPtr->at(customerIndex);

    assert(MAX_PROB > 0);
    auto randVal = uDist(randGen);
    currentProb = randVal % MAX_PROB;
    assert(currentProb >= 0 && currentProb < 100);

    cout << iteration << ", " << global_time_sec << ", " << timeadvance << ", "
         << nanos << ", " << currentCustomer->getID() << ", "
         << getStateName(currentCustomer->getState()) << " ("
         << currentCustomer->getState() << ")" << ", " << timestamp << ", "
         << nanos_day << endl;

    switch (currentCustomer->getState()) {
    case customer::IDLE:
      if (currentProb <= cancelProb)
        currentCustomer->goCancel();
      else if (currentProb <= logoutProb)
        currentCustomer->logOut();
      else if (currentProb <= shoppingProb)
        currentCustomer->goShopping();
      // else do nothing, remains IDLE

      break;
    case customer::SHOPPING:
      if (currentCustomer->getTotOrders() < orderLimit) {
        status = reqstatus::RequestStatus{};
        if ((numProducts = prodWindowPtr->getNumProducts()) == 0) {
          cout << "Product window before refresh" << endl;
          prodWindowPtr->printProductWindow();
          getProdsRes = getProducts(c2r, req_stream, reply_stream, restockQty,
                                    prodWindowPtr, status, CONSUMER_GROUP);
          if (getProdsRes == EXIT_FAILURE) {
            cerr << __func__ << "(): getProducts: " << status.msg << endl;
            if (status.code == reqstatus::SERVER_DOWN) {
              cerr << "Server response timeout, "
                      "server maybe down..."
                   << endl;
              freeRedisStreams();
            }
            nextIter();
            continue;
          }
          cout << "Product window after refresh" << endl;
          prodWindowPtr->printProductWindow();
        }

        // if product window is still empty then skip this iteration
        if ((numProducts = prodWindowPtr->getNumProducts()) == 0) {
          nextIter();
          continue;
        }
        status = reqstatus::RequestStatus{};

        // RANDOM SELECTION OF PRODUCTS
        cout << "Starting random selection"
             << " of products to order..." << endl;
        cout << "Product window before ordering" << endl;
        prodWindowPtr->printProductWindow();
        numProducts = prodWindowPtr->getNumProducts();
        assert(numProducts > 0);
        assert(maxProducts > 0);
        numProductsToOrder =
            (uDist(randGen) % min(numProducts, (ulong)maxProducts)) + 1;
        cout << "Number of products to order: " << numProductsToOrder << endl;

        orderPtr->clear();
        for (size_t i = 0; i < numProductsToOrder; ++i) {
          prodID = prodWindowPtr->getKthProductID(uDist(randGen) % numProducts);

          totProdQty = prodWindowPtr->getProductQuantity(prodID);
          assert(totProdQty > 0);
          prodQty = (uDist(randGen) % totProdQty) + 1;
          orderPtr->insert({prodID, prodQty});

          cout << "Added " << prodQty << " pieces of product id=" << prodID
               << " to order basket..." << endl;
        }

        if (orderPtr->size() > 0) {
          cout << "----- Customer=" << currentCustomer->getID()
               << " order -----" << endl;
          for (auto p : *orderPtr) {
            cout << p.first << ": " << p.second << endl;
          }
          cout << "----------------------------" << endl;

          status = reqstatus::RequestStatus{};
          orderRes = currentCustomer->order(c2r, req_stream, reply_stream,
                                            orderPtr, status, CONSUMER_GROUP);

          if (orderRes == EXIT_FAILURE) {
            cerr << "Customer=" << currentCustomer->getID()
                 << " order failed: code: " << status.code << " " << status.msg
                 << endl;

            if (status.code == reqstatus::SERVER_DOWN) {
              cerr << "Server response timeout, "
                      "server maybe down..."
                   << endl;
              freeRedisStreams();
            }
          } else {
            cout << "Customer=" << currentCustomer->getID()
                 << " order was successful " << endl;
            status = reqstatus::RequestStatus{};
            // Decreases the quantities of products
            // in the product window from the order
            for (auto row : *orderPtr) {
              if (!prodWindowPtr->decreaseQty(row.first, row.second)) {
                cerr << __func__ << "(): LINE: " << __LINE__
                     << " failed to decrease product qty"
                     << " in product window" << endl;
                redisFree(c2r);
                exit(EXIT_FAILURE);
              }

              // removes product from product window if qty <= 0
              if (prodWindowPtr->getProductQuantity(row.first) <= 0) {
                prodWindowPtr->removeProduct(row.first);
              }
            }
          }

          cout << "Product window after ordering" << endl;
          prodWindowPtr->printProductWindow();
          orderPtr->clear();
          ++totalOrders;
        } else {
          cerr << __func__ << "(): LINE: " << __LINE__
               << " order has no products, something went wrong"
               << " with the random product selection" << endl;
        }

      } else {
        // Order limit reached by customer, terminate customer
        currentCustomer->logOut();
        break;
      }

      if (currentProb <= cancelProb)
        currentCustomer->goCancel();
      else if (currentProb <= logoutProb)
        currentCustomer->logOut();
      break;
    case customer::CANCELLING:
      cout << "Customer=" << currentCustomer->getID()
           << ", tot order: " << currentCustomer->getTotOrders() << endl;

      if (currentCustomer->getTotOrders() > 0) {
        status = reqstatus::RequestStatus{};
        // cancels randomly an order.
        if (currentCustomer->cancelOrder(c2r, req_stream, reply_stream, status,
                                         CONSUMER_GROUP) == EXIT_SUCCESS) {
          ++totalCancelledOrders;
          cout << "Customer=" << currentCustomer->getID()
               << " cancelled an order successfully..." << endl;
        } else {
          cerr << "Customer=" << currentCustomer->getID()
               << " cancel order failed: code: " << status.code << " "
               << status.msg << endl;

          if (status.code == reqstatus::SERVER_DOWN) {
            cerr << "Server response timeout, "
                    "server maybe down..."
                 << endl;
            freeRedisStreams();
          }
        }
        status = reqstatus::RequestStatus{};
      }

      if (currentProb <= logoutProb)
        currentCustomer->logOut();
      else
        currentCustomer->goIdle();
      break;
    case customer::TERMINATED:
      --customerPoolSize;
      customerPoolPtr->erase(customerPoolPtr->begin() + customerIndex);
      break;
    default:
      // unreachable segment of code
      break;
    } // end of switch
    nextIter();
  } while (1);
}
