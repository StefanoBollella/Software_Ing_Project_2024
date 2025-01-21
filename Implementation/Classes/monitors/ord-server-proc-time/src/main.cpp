#include "main.h"
#include <optional>
#include <string>

#define SQLCMD_SIZE 5000
#define TIMESTAMP_SIZE 200
#define MIN_ARG 2


/* This monitor should be run according to this order:
 * 1. Log DB
 * 2. This monitor
 * so that it will be able to monitor all orders from the start of the system
 * and not only those that were after the start of this monitor */

// input: threshold in nanoseconds
int main(int argc, char *argv[]) {
    if (argc < MIN_ARG) {
        std::cerr << "main(): usage: ./exec threshold_ns" << std::endl;
        return 1;
    }
    char sqlcmd[SQLCMD_SIZE], buf[TIMESTAMP_SIZE];
    char *user_id, *req_id, *serverPid, *duration_nanos, *lastReadTimestamp;
    unsigned long iteration = 0, threshold;
    long lastReadTimestampNanos;
    int totTuples = 0;
    pid_t pid;
    Con2DB logdb("localhost", "5432", "logdb_usr", "47002", "logdb");
    PGresult *res;
    std::string alert;

    threshold = std::stoul(argv[1]);

    pid = getpid();
    std::cout << "---Server order processing time monitor---" << std::endl;
    std::cout << "Start main with pid " << pid <<
        " and threshold of " << threshold <<
        " ns" << std::endl;

    std::cout << "Format: " <<
        "time step , " <<
        "global time in second, " <<
        "elapsed time in sec, " <<
        "present time in nanosec, " <<
        "timestamp, " <<
        "epoch" << std::endl;

    init_time();
    nanos = get_nanos();
    nanos_day = get_day_nanos(buf);
    lastReadTimestamp = buf;
    lastReadTimestampNanos = nanos_day;

    while (1) {
        std::cout << iteration << ", " <<
            global_time_sec << ", " <<
            timeadvance << ", " <<
            nanos << ", " <<
            buf << ", " <<
            nanos_day << std::endl;

        nanos_day = get_day_nanos(buf);

        if (snprintf(
            sqlcmd,
            sizeof(sqlcmd),
            "WITH data AS (SELECT \'%s\'::timestamp as ts, %lu as ns) "
            "SELECT op.req_id, op.user_id as usr_id, "
            "lt.pid, lt.timestamp, lt.nanosec, lt.duration_nanos "
            "FROM operation op, log_table lt, data d "
            "WHERE op.id = lt.operation_id "
            "AND op.op_type = 'order' "
            "AND ((lt.timestamp > d.ts) "
            "     OR (lt.timestamp = d.ts AND lt.nanosec > d.ns)) "
            "AND op.component = 'CUSTOMER' "
            "ORDER BY (lt.timestamp, lt.nanosec) ASC",
            lastReadTimestamp,
            lastReadTimestampNanos) > (int)sizeof(sqlcmd)) {
            std::cerr << "SQL cmd buffer insufficient space" << std::endl;
            exit(EXIT_FAILURE);
        }

        res = logdb.ExecSQLtuples(sqlcmd);

        totTuples = PQntuples(res);
        if (totTuples > 0) {
            for (int i = 0; i < totTuples; ++i) {
                serverPid = PQgetvalue(res, i, PQfnumber(res, "pid"));
                user_id = PQgetvalue(res, i, PQfnumber(res, "usr_id"));
                req_id = PQgetvalue(res, i, PQfnumber(res, "req_id"));
                duration_nanos =
                    PQgetvalue(res, i, PQfnumber(res, "duration_nanos"));
                lastReadTimestamp =
                    PQgetvalue(res, i, PQfnumber(res, "timestamp"));
                lastReadTimestampNanos =
                    std::stoull(PQgetvalue(res, i, PQfnumber(res, "nanosec")));

                alert = "Customer server "
                    + std::string(serverPid, strlen(serverPid))
                    + " processing of order by customer "
                    + std::string(user_id, strlen(user_id))
                    + " with request id: "
                    + std::string(req_id, strlen(req_id))
                    + "\nTime (ns): "
                    + std::string(duration_nanos, strlen(duration_nanos));


                if (std::stoull(duration_nanos) > threshold) {
                    alert = "> ALERT: " + alert + " > "
                        + std::to_string(threshold)
                        + ", exceeded threshold!";
                    log2db(
                        logdb, "order", std::nullopt, std::nullopt,
                        std::make_optional(std::string(req_id, strlen(req_id))),
                        "MONITOR", "monitor::ord-server-proc-time",
                        "WARNING", pid, std::nullopt, std::make_optional(alert)
                    );
                }
                std::cout << alert << "\n" << std::endl;
            }
        }

        // half a second sleep
        micro_sleep(500000);
        update_time();
        ++iteration;
    }
}
