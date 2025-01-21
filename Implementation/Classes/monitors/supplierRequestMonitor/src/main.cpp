#include "main.h"

#define MAX_ARGS 3
#define SQLCMD_SIZE 5000
#define TIMESTAMP_SIZE 200

int main(int argc, char *argv[]) {

    if(argc < MAX_ARGS){
        std::cerr << "Usage: ./exec interval_microseconds max_requests" << std::endl;
        return 1;
    }
    long interval_microseconds = std::stol(argv[1]);  
    long long interval_nanoseconds = interval_microseconds * 1000;  
    int min_requests = std::stoi(argv[2]); 
    char sqlcmd[SQLCMD_SIZE], buf[TIMESTAMP_SIZE];
   
    unsigned long iteration = 0;
    int totTuples = 0;
    int request_count = 0;  
    long first_nanosec = 0;  //Nanoseconds associated with the first timestamp
    pid_t pid;
    std::string message = ""; 
    
    Con2DB logdb("localhost", "5432", "logdb_usr", "47002", "logdb");
    PGresult *res;

    pid = getpid();
    std::cout << "---Supplier Request Monitor---" << std::endl;
    std::cout << "Monitoring supplier requests every " << interval_microseconds 
              << " microseconds (" << interval_nanoseconds / 1000000000.0 << " seconds)." << std::endl;

    //Retrieves the first timestamp and nanoseconds in the log database associated with the SUPPLIER component
    if (snprintf(
        sqlcmd, 
        sizeof(sqlcmd), 
        "SELECT lt.timestamp AS first_timestamp, lt.nanosec AS first_nanosec "
        "FROM log_table lt, operation op "
        "WHERE op.component = 'SUPPLIER' "
        "AND lt.operation_id = op.id "
        "AND lt.timestamp = (SELECT MIN(lt2.timestamp) "
                            "FROM log_table lt2, operation op2 "
                            "WHERE op2.component = 'SUPPLIER' "
                            "AND lt2.operation_id = op2.id);") > (int)sizeof(sqlcmd)) {
    std::cerr << "SQL command buffer insufficient space" << std::endl;
    exit(EXIT_FAILURE);
}

    res = logdb.ExecSQLtuples(sqlcmd);
    totTuples = PQntuples(res);

    if (totTuples > 0) {
        char *first_timestamp = PQgetvalue(res, 0, PQfnumber(res, "first_timestamp"));
        char *first_nanosec_str = PQgetvalue(res, 0, PQfnumber(res, "first_nanosec"));
        first_nanosec = std::stol(first_nanosec_str);

        printf("%-10s %-15s %-15s %-20s %-25s %-10s\n",  
       "time step", "global_time [s]", "elapsed [s]", "present_time [ns]", "timestamp", "nanos");
        
        init_time();
        nanos = get_nanos();

        while (1) {
            ++iteration;
            nanos_day = get_day_nanos(buf);
            
            printf("%-10ld %-15.5lf %-15.5lf %-20ld %-25s %-10ld", 
                   iteration, global_time_sec, timeadvance, nanos, buf, nanos_day);
           
            long long start_nanosec = (iteration - 1)* interval_nanoseconds;
            long long end_nanosec = iteration * interval_nanoseconds;

           if (snprintf(
                  sqlcmd, 
                  sizeof(sqlcmd), 
                  "SELECT COUNT(*) as request_count "
                  "FROM log_table lt, operation op "
                  "WHERE op.component = 'SUPPLIER' " 
                  "AND lt.operation_id = op.id "
                  "AND (EXTRACT(EPOCH FROM lt.timestamp) * 1000000000 + lt.nanosec) >= "
                  "(EXTRACT(EPOCH FROM TIMESTAMP '%s') * 1000000000 + %ld + %lld) "
                  "AND (EXTRACT(EPOCH FROM lt.timestamp) * 1000000000 + lt.nanosec) < "
                  "(EXTRACT(EPOCH FROM TIMESTAMP '%s') * 1000000000 + %ld + %lld);",
                  first_timestamp, first_nanosec, start_nanosec,
                  first_timestamp, first_nanosec, end_nanosec) > (int)sizeof(sqlcmd)) {
              std::cerr << "SQL command buffer insufficient space" << std::endl;
              exit(EXIT_FAILURE);
          }

          
            //std::cout << "SQL Command: " << sqlcmd << std::endl;
            res = logdb.ExecSQLtuples(sqlcmd);
            totTuples = PQntuples(res);

            if(totTuples > 0){
                char *request_count_str = PQgetvalue(res, 0, PQfnumber(res, "request_count"));
                request_count = std::stoi(request_count_str);
                printf(" request count %d ",request_count); 
                
                //If the number of requests is less than the minimum allowed min_requests, launch a warning
                if(request_count < min_requests){
                      
                     message += "ALERT: low number of requests processed "
                                + std::to_string(request_count) 
                                + " < " + std::to_string(min_requests);
                  
                    printf( " %-10s ",message.c_str()); 
                
                   log2db(logdb, "supplier-activity", std::nullopt, std::nullopt, std::nullopt,
                          "MONITOR", "monitor::supplierRequestMonitor", "WARNING",pid, std::nullopt,
                          std::make_optional(message)); 
                }
            }
            else{
                std::cerr << "Unexpected result: no tuples found in the result set." << std::endl;
            }
        
        
        
        
        micro_sleep(interval_microseconds);
        update_time();
        printf("\n");
      }//while()
    } 
    else {
        std::cerr << "No initial timestamp found for SUPPLIER component." << std::endl;
    }

    return 0;
}
