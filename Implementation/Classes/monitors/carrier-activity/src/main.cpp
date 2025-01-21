#include "main.h"
#include <optional>

#define SQLCMD_SIZE 5000
#define TIMESTAMP_SIZE 200
#define MIN_ARG 4

// input: sleep time in microseconds
int main(int argc, char *argv[]) {
    
    if (argc < MIN_ARG) {
        std::cerr << "main(): usage: ./exec sleep_time_micros threshold count_get_orders" << std::endl;
        return 1;
    }
    char sqlcmd[SQLCMD_SIZE], buf[TIMESTAMP_SIZE];
    unsigned long iteration = 0, sleep_time;
    int totTuples = 0, previousTotal = 0, threshold;
    pid_t pid;
    bool count_get_orders = true;
    Con2DB logdb("localhost", "5432", "logdb_usr", "47002", "logdb");
    PGresult *res;

    std::string message="";

    // should be 10 seconds
    sleep_time = std::stoul(argv[1]);

    // should be 2, so if 2 more operation is recorded it is ok.
    threshold = std::stoul(argv[2]);

    // if false the monitor does not count getting orders as an activity
    if(std::stoul(argv[3]) == 1){
        count_get_orders = true;
    }else if(std::stoul(argv[3]) == 0){
        count_get_orders = false; 
    }
    // count_get_orders = std::stoul(argv[3]);

    pid = getpid();
    std::cout << "---Server carrier activity monitor---" << std::endl;
    std::cout << "Start main with pid " << pid << 
    " and sleep_time of " << sleep_time << "microsec" 
    ", threshold: " << threshold << ". count getting orders as activity? " << count_get_orders << std::endl;

    std::cout << "Format: " <<
        "time step , " <<
        "global time in second, " <<
        "elapsed time in sec, " <<
        "present time in nanosec, " <<
        "timestamp, " <<
        "epoch" << std::endl;

    init_time();
    nanos = get_nanos();

    update_time();
    ++iteration;

    do {

        nanos_day = get_day_nanos(buf);

        std::cout << iteration << ", " <<
            global_time_sec << ", " <<
            timeadvance << ", " <<
            nanos << ", " << 
            buf << ", " <<
            nanos_day << std::endl;

        if(count_get_orders){
            if (snprintf(
                sqlcmd, 
                sizeof(sqlcmd),
                "SELECT * "
                "FROM operation "
                "WHERE component = 'CARRIER' "
            ) > (int)sizeof(sqlcmd)){
                std::cerr << "SQL cmd buffer insufficient space" << std::endl;
                exit(EXIT_FAILURE);            
            }
        }else{
            if (snprintf(
                sqlcmd, 
                sizeof(sqlcmd),
                "SELECT * "
                "FROM operation "
                "WHERE component = 'CARRIER' AND NOT (req_type = 'get-order' ) "
            ) > (int)sizeof(sqlcmd)){
                std::cerr << "SQL cmd buffer insufficient space" << std::endl;
                exit(EXIT_FAILURE);            
            }            
        }

        res = logdb.ExecSQLtuples(sqlcmd);

        totTuples = PQntuples(res);

        // first iteration just saves the baseline total of operations
        if(iteration == 1){
            previousTotal = totTuples;
            std::cout << "MESSAGE: initializing action count. " << std::endl; 
            micro_sleep(sleep_time);
            update_time();
            ++iteration;           
            continue;
        }
        message = "";
        // if true then no operations were done in the sleep time: send warning!
        if(previousTotal == totTuples){
            std::cout << "ALERT: no actions were taken in the sleep time! " << 
            " total actions: " << totTuples << std::endl;
            message += "ALERT: no actions were taken in the sleep time! ";
        }   else if((totTuples - previousTotal) < threshold )  {
            std::cout << "ALERT: actions were taken during sleep time. " << 
            " amount of actions taken: " << totTuples - previousTotal << 
            ". less than the threshold! " << std::endl;
            message += "ALERT: actions were taken during sleep time, but less than threshold ";
        }   else    {
            std::cout << "MESSAGE: actions were taken during sleep time. " << 
            " amount of actions taken: " << totTuples - previousTotal << std::endl;  
            message += "MESSAGE: actions were taken during sleep time. ";
        }

        // logging into log2db 
        log2db(
            logdb,"carrier-activity", std::nullopt,std::nullopt,std::nullopt,
            "MONITOR", "monitor::carrier-activity","WARNING",pid,std::nullopt, std::make_optional(message)
        );

        previousTotal = totTuples;

        // sleeps for sleep_time
        micro_sleep(sleep_time);
        update_time();
        ++iteration;

    } while(1);

    return 0;
}
