#ifndef carrier_h
#define carrier_h

#include<queue>

#include "main.h"

// redis
#include "../../Carrier-shared/src/RequestStatus.h"
#include "../../Carrier-shared/src/request_formats.h"
#include "../../Carrier-shared/src/utils.h"

#include "../../con2redis/src/con2redis.h"
#include "../../con2redis/src/local.h"

enum CarrierStates {delivering, ready, waiting}; 

class Carrier
{
	private:
	
		CarrierStates state;
		int Pmod;
		unsigned long id;
		//queue<unsigned long> orders; //
		//q.push(thing), a=q.pop()-> void, q.size()-> int, .front()-> first element, .back(),.empty()->true/false, 
		
	public:
		
		std::queue<unsigned long> orders; // is it correct to make this public?
		
		// Carrier constructor. requires the id.
		Carrier(unsigned long id);
		// get the Carrier id.
		unsigned long getId();
		// get the current state of the Carrier.
		CarrierStates getState();
		
		// get the Pmod of Carrier. this is a modifier.
		int getPmod();
		// set the value of the modifier Pmod.
		void setPmod(int value);
		
		// switch the state of the Carrier into delivering.
		void goDelivering();
		// switch the state of the Carrier into ready.
		void goReady();
		// switch the state of the Carrier into waiting.
		void goWaiting();
		
		// get up to MAX_ORDERS orders, insert them in the queue, return based on success of operation
		int getOrders(redisContext *c2r, const char *reqStream, const char *replyStream, unsigned char MAX_ORDERS, 
						reqstatus::RequestStatus &status, const char* carrierGrp);

		// deliver the current order
		int deliverOrder(
			redisContext* c2r, const char *requestStream, const char *replyStream, reqstatus::RequestStatus& status, const char *carrierGrp, unsigned long order_id
		);
		
		// lose the current order
		int loseOrder(
			redisContext* c2r, const char *requestStream, const char *replyStream, reqstatus::RequestStatus& status, const char *carrierGrp, unsigned long order_id
		);
		
};

#endif //carrier_h

