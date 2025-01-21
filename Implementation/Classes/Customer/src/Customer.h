#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <climits>
#include <memory>
#include <random>
#include <unordered_map>
#include <vector>

#include "../../Customer-shared/src/RequestStatus.h"
#include "../../con2redis/src/con2redis.h"
#include "../../con2redis/src/local.h"

#define RAND_CANCEL -1

namespace customer {

enum CustomerState { SHOPPING, IDLE, CANCELLING, TERMINATED };

class Customer {
  private:
    ulong orderLimit;
    ulong id;
    CustomerState currentState;

    // allows random order cancellation
    std::vector<ulong> orders;

  public:
    Customer(ulong id, ulong orderLimit);

    ulong getID() { return this->id; }

    CustomerState getState() { return this->currentState; }

    int order(
        redisContext* c2r, const char* requestStream,
        const char* replyStream,
        std::unique_ptr<std::unordered_map<ulong, ulong>>& products,
        reqstatus::RequestStatus& status,
        const char* consumerGrp
    );

    int cancelOrder(
        redisContext *c2r, const char *requestStream,
        const char *replyStream, reqstatus::RequestStatus& status,
        const char *consumerGrp, int orderInd = RAND_CANCEL
    );

    int getIthOrder(ulong orderInd) { return this->orders.at(orderInd); }

    // order IDs are zero-index based so nextOrderID is the total
    ulong getTotOrders() { return this->orders.size(); }

    /* this terminates the customer even if it has still pending orders, just
     * like in real life this makes some customers after logging out not able to
     * do other operations it's orders will still get "fulfilled" by the
     * carrier. */
    void logOut() { this->currentState = TERMINATED; }
    void goIdle() { this->currentState = IDLE; }
    void goShopping() { this->currentState = SHOPPING; }
    void goCancel() { this->currentState = CANCELLING; }
};

}  // namespace customer

#endif  // CUSTOMER_H
