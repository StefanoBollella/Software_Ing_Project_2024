#include <queue>

#include "Carrier.h"

Carrier::Carrier(unsigned long id){
	
	// this->orders;
	this->Pmod = 0;
	this->state = waiting;
	this->id = id;
	
}

unsigned long Carrier::getId(){ 
	return this->id; 
}

CarrierStates Carrier::getState(){ 
	return this->state;
}

int Carrier::getPmod(){ 
	return this->Pmod; 
}

void Carrier::setPmod(int value){ 
	this->Pmod = value; 
} // this should work
	
void Carrier::goDelivering(){ 
	this->state = delivering; 
}

void Carrier::goReady(){ 
	this->state = ready;
}
	
void Carrier::goWaiting(){ 
	this->state = waiting; 
}




