#include "Customer.h"

namespace customer {

Customer::Customer(unsigned long id, unsigned long orderLimit) {
	this->currentState = IDLE;
	this->id = id;
	this->orderLimit = (orderLimit == 0) ? ULONG_MAX : orderLimit;
}

} // namespace customer
