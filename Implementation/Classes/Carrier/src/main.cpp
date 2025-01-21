#include "main.h"

using namespace std;

namespace{

long int iteration = -1;
long int deliveredOrders = 0; 
long int lostOrders = 0;
int pid = getpid();
// ----- redis ----- //
redisContext* c2r;
const char HOST_NAME[] = "localhost";
const char CARRIER_GROUP[] = "carrier-grp-0";
const uint PORT_NUM = 6379;
char* req_stream_main;
char* reply_stream_main;

inline void signalHandlerPrintInfo(int signal) {

    cout << "\nTerminated by signal SIGINT" << endl;
    cout << "Total orders delivered: " << deliveredOrders << endl;
    cout << "Total lost orders: " << lostOrders << endl;
    cout << "Total iterations: " << iteration << endl;

    exit(EXIT_SUCCESS);
}

inline void initRedisStreams() {
    cout << "main(): pid: " << pid << " connecting to redis ..." << endl;
    c2r = redisConnect(HOST_NAME, PORT_NUM);

    if (c2r == nullptr || c2r->err) {
        if (c2r)
            cerr << "initRedisStreams(): Error connecting to redis: \n" << c2r->errstr << endl;
        else
            cerr << "initRedisStreams(): Can't allocate redis context" << endl;
        exit(EXIT_FAILURE);
    }
    cout << "main(): pid: " << pid << " connected to redis ..." << endl;

}

inline void freeRedisStreams() {
    int attempts = 10;

    while (attempts > 0
        && (streamExists(c2r, req_stream_main) || streamExists(c2r, reply_stream_main))) {
        RedisCommand(c2r, "DEL %s", req_stream_main);
        RedisCommand(c2r, "DEL %s", reply_stream_main);
        --attempts;
    }

    if (!streamExists(c2r, req_stream_main)) {
        cout << "main(): pid: " << pid << " freed redis stream "
             << req_stream_main << "..." << endl;
    }

    if (!streamExists(c2r, reply_stream_main)) {
        cout << "main(): pid: " << pid << " freed redis stream "
             << reply_stream_main << "..." << endl;
    }

    // we don't clean the redisContext (c2r) since
    // we only want to delete the redis streams
}

} // namespace ends here

int main(int argc, char** argv)	{

	// -------------------- startup checks -------------------- //

	const char MIN_ARGS = 4;
	assert(argc >= MIN_ARGS);

	// -------------------- loading constants -------------------- //

	cout << "starting carrier with pid: " << pid << endl;

	// changes the signal handler of SIGINT to signalHandlerPrintInfo
    signal(SIGINT, signalHandlerPrintInfo);

	optional<YAML::Node> configParams;
	try 
	{
		// load parameters from config. file
		configParams = loadConfigFile(argv[1]);
	}
	catch (const invalid_argument& e)
	{
		cerr << "Exception caught: " << e.what() << endl;
	}

	if (!configParams.has_value())
		return EXIT_FAILURE;	

	const unsigned short P_LOST = (*configParams)["P_LOST"].as<unsigned short>();
	const unsigned short P_DELIVER = (*configParams)["P_DELIVER"].as<unsigned short>();
	const unsigned short P_END_WAIT = (*configParams)["P_END_WAIT"].as<unsigned short>();
	const unsigned short WAIT_MOD = (*configParams)["WAIT_MOD"].as<unsigned short>();
	const unsigned short MAX_ORDERS = (*configParams)["MAX_ORDERS"].as<unsigned short>();
	const unsigned short MAX_CARRIERS = (*configParams)["MAX_CARRIERS"].as<unsigned short>();
	const unsigned short Q_GEN_RATIO = (*configParams)["Q_GEN_RATIO"].as<unsigned short>();
	const unsigned short N_INTERVALS = (*configParams)["N_INTERVALS"].as<unsigned short>();
	
	assert(P_LOST < P_DELIVER && "fatal: P_LOST not < P_DELIVER");
	assert(P_DELIVER <= 100 && P_DELIVER > 0 && "fatal: P_DELIVER not <= 100 or P_DELIVER <= 0");
	assert(P_END_WAIT <= 100 && P_END_WAIT > 0 && "fatal: P_END_WAIT > 100 or P_END_WAIT <= 0");
	assert(N_INTERVALS <= MAX_CARRIERS && "fatal: N_INTERVALS is too big (> MAX_CARRIERS)");
	assert(MAX_ORDERS > 0 && "fatal: MAX_ORDERS is <= 0");

	// -------------------- random numbers setup -------------------- //

	typedef mt19937 rng_type;

	random_device rand_dev;
	uniform_int_distribution<rng_type::result_type> uDist(1, 100);
	rng_type randGen(rand_dev());

	// -------------------- redis and stuff -------------------- //

	//main
	req_stream_main = argv[2];
	reply_stream_main = argv[3];
	//sub
	// char* req_stream_sub = argv[4];
	// char* reply_stream_sub = argv[5];

	reqstatus::RequestStatus status = reqstatus::EmptyReqStatus;

	cout << "main request stream: " << req_stream_main << endl;
	cout << "main reply stream: " << reply_stream_main << endl;
	// cout << "sub request stream: " << req_stream_sub << endl;
	// cout << "sub reply stream: " << reply_stream_sub << endl;

	// ----- connecting to redis ----- //

	cout << "connecting to redis... " << endl;

	initRedisStreams();

	cout << "connected to redis" << endl;

	// --------------------  -------------------- //	

	queue<Carrier> carrierQ;
	Carrier carrier(0); // is it correct for the temp carrier to be like this? it's just used to point to a carrier in queue
	/*
	// THIS IS A TEMPORARY WAY OF FILLING THE QUEUE
	for (unsigned long i = 0; i < MAX_CARRIERS; ++i){
		Carrier tempC(i);
		carrierQ.push(tempC);
	}*/

    // long int iteration = -1;
	// long int deliveredOrders = 0; 
	// long int lostOrders = 0;
    int slice = MAX_CARRIERS /(double)N_INTERVALS;
    bool full_gen = false;

	char buf[200];
	init_time();
	nanos = get_nanos();
	// msleep(100);
	update_time();
	cout << "time: " << global_time_sec << endl;

	cout << "Format: " << 
        "time step, " << 
        "global time in second, " <<
        "elapsed time in sec, " << 
        "present time in nanosec, " <<
        "carrierID, " <<
        "state, " <<
        "timestamp, " << 
        "epoch" << endl;

	while(!streamExists(c2r, req_stream_main) || !streamExists(c2r, reply_stream_main)) {
		cout << "streams are not active. waiting... " << endl;
		micro_sleep(500000);
		update_time();
	}

	// this while could be time-based.
	while(true)	{ 

		iteration++;
		nanos_day = get_day_nanos(buf);

		// checking that the stream exist 
		if(!streamExists(c2r, req_stream_main) || !streamExists(c2r, reply_stream_main)) {
			cout << "streams are not active. waiting... " << endl;
			micro_sleep(500000);
			update_time();
			continue;
		}

		if( (iteration % Q_GEN_RATIO==0 || carrierQ.size() == 0) && !full_gen ){

            if ((int)carrierQ.size() + slice >= MAX_CARRIERS){ // il compito di questo if è assicurarsi di non generare più oggetti di MAX
                slice = MAX_CARRIERS - carrierQ.size();
                full_gen = true;
            } 

            cout << "iteration " << iteration << ": generating " << slice << " carriers" << endl;
            
			vector<ulong> tempcarIDs;

			if(registerCarrier(
				c2r, req_stream_main, reply_stream_main, CARRIER_GROUP, status, slice, tempcarIDs
			) == EXIT_FAILURE ){
				cerr << "error registering carriers. code: " << status.code << "; message: " << status.msg << endl;
				if (status.code == reqstatus::SERVER_DOWN) {
                    cerr << "Server response timeout, the server may be down" << endl;
                    freeRedisStreams();
                }
			}

			for(unsigned long id:tempcarIDs){
				Carrier tempC(id);
				carrierQ.push(tempC);
			}
			tempcarIDs.clear();
            continue; // nel caso in cui siamo in generazione oggetti non lavoriamo con la queue. questo non è necessario, si puo levare se si vuole.
        }
		
		// safety check
		if(carrierQ.size() == 0){
			cout << "the carrier queue is empty... " << endl;
			micro_sleep(500000);
			update_time();
			continue;
		}

		// the current carrier is taken, put back into the queue, and ready for his turn.
		carrier = carrierQ.front();

		// message of the iteration
		cout << "iteration: " << iteration << ", " <<
				global_time_sec << ", " <<
				timeadvance << ", " <<
				nanos << ", " <<
				carrier.getId() << ", " <<
				carrier.getState() << ", " <<
				buf << ", " <<
				nanos_day << endl;

		// vars
		int chance;

		switch(carrier.getState()){ // current carrier
			
			/*
			IMAGE:  0      P_LOST     P_DELIVER          100
                    ^  lose  ^  deliver   ^  do-nothing   ^
				    |--------|------------|---------------|
			*/
			case delivering:
				
				// cout << "Carrier " << carrier.getId() << " is delivering..." << endl;

				if( carrier.orders.empty() ) {
					carrier.goWaiting();
		
				} else	{
		
					unsigned long current_order = carrier.orders.front();
					chance = uDist(randGen); // random [1;100]
		
					if (chance <= P_LOST){
			
						if( carrier.loseOrder(c2r, req_stream_main, reply_stream_main, status, CARRIER_GROUP, current_order) == EXIT_FAILURE ){ 
							// send error message
							cerr << "error losing order. code: " << status.code << "; message: " << status.msg << endl;
							if (status.code == reqstatus::SERVER_DOWN) {
                    			cerr << "Server response timeout, the server may be down" << endl;
                    			freeRedisStreams();
                			}
						}
						
						lostOrders++;
						update_time();
						cout << "Carrier " << carrier.getId() << " lost order " << current_order << " TIME: " << global_time_sec << endl;
		
					} else if (chance <= P_DELIVER){ // the previous "if" already checked that chance is not smaller than P_LOST
			
						if( carrier.deliverOrder(c2r, req_stream_main, reply_stream_main, status, CARRIER_GROUP, current_order) == EXIT_FAILURE ){
							// send error message
							cerr << "error delivering order. code: " << status.code << "; message: " << status.msg << endl;
							if (status.code == reqstatus::SERVER_DOWN) {
                    			cerr << "Server response timeout, the server may be down" << endl;
                    			freeRedisStreams();
                			}
						}

						deliveredOrders++;
						update_time();
						cout << "Carrier " << carrier.getId() << " delivered order " << current_order << " TIME: " << global_time_sec << endl;
			
					} else {
						break; // because we dont want to pop an order that hasnt been "used"
					}
		
					carrier.orders.pop();
					// carrier.goDelivering(); // redundant
		
				}
				
				break;
				
			case ready:
			
				// cout << "Carrier " << carrier.getId() << " is ready and getting orders to deliver." << endl;

				// refill the orders to deliver, if the queue is empty.
				if(carrier.orders.empty()) {
					if(carrier.getOrders(c2r, req_stream_main, reply_stream_main, MAX_ORDERS, status, CARRIER_GROUP) == EXIT_FAILURE){
						cerr << "error getting orders. code: " << status.code << "; message: " << status.msg << endl;
						if (status.code == reqstatus::SERVER_DOWN) {
                    		cerr << "Server response timeout, the server may be down" << endl;
                    		freeRedisStreams();
                		}
						// if the function to get orders fails

					}
				}	
				
				if ( carrier.orders.empty() )	{ // if after taking orders the queue is still empty, none were taken, go back to waiting.
					cout << "Carrier " << carrier.getId() << ": no orders were taken or found.. back to waiting" << endl;
					carrier.goWaiting();
				}	else {
					carrier.goDelivering();
				}
				break;
				
			/*
			IMAGE:  0        P_END_WAIT    WAIT_MOD       100
			        ^  end-wait  ^    same    ^            ^
				    |------------|------------>------------|
			*/
			case waiting:

				// cout << "Carrier " << carrier.getId() << " is waiting..." << endl;

				chance = uDist(randGen); // dandom [1;100]
				if (chance <= (P_END_WAIT + carrier.getPmod())){
					carrier.setPmod(0); 
					carrier.goReady();
				} else	{
					carrier.goWaiting(); // redundant
					int value = ( carrier.getPmod() + (int)( (100 - (P_END_WAIT + carrier.getPmod()) )/(double)WAIT_MOD ) );
					carrier.setPmod(value);
			
					/*
					//should i do this? i should calculate if it's even possible to go over 100, might not be.
					while (value + P_END_WAIT > 100){
						value--;
					}
					carrier.setPmod(value);
					*/
					// through testing i have never seen this assert in action. remove?
					assert(value + P_END_WAIT <= 100 && "probability of exiting wait status exceeded 100");
					
				}
				
				break;
			
			default:
				// unreachable
				break;
		}
		
		// this is for now here, since front() gives a read/write to the top, if i pop immediately it does not work
		carrierQ.pop();
		
		carrierQ.push(carrier);

		micro_sleep(500000);

		update_time();

	} // while(true) 
	
	return 0;
}
