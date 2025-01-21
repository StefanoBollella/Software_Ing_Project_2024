#include "main.h"
#include <optional>

#define SQLCMD_SIZE 512
#define TIMESTAMP_SIZE 200
#define MIN_ARG 2

int main(int argc, char *argv[]) {
    if (argc < MIN_ARG) {
        std::cerr << "main(): usage: ./exec threshold_MB" << std::endl;
        return 1;
    }

    char sqlcmd1[SQLCMD_SIZE], sqlcmd2[SQLCMD_SIZE], buf[TIMESTAMP_SIZE];
    Con2DB logdb("localhost", "5432", "logdb_usr", "47002", "logdb");
    pid_t pid;
    unsigned long iteration, diskSpaceUsed, threshold;
    PGresult *res;
    std::string alert;

    iteration = 0;
    threshold = std::stoul(argv[1]);

    pid = getpid();
    std::cout << "---DB disk usage monitor---" << std::endl;
    std::cout << "Start main with pid " << pid <<
        " with " << threshold << " MB" << std::endl;

    std::cout << "Format: " <<
        "time step , " <<
        "global time in second, " <<
        "elapsed time in sec, " <<
        "present time in nanosec, " <<
        "timestamp, " <<
        "epoch" << std::endl;

    init_time();
    nanos = get_nanos();

    snprintf(
        sqlcmd1,
        sizeof(sqlcmd1),
        "SELECT pg_database_size('ecommerce_db')"
    );

    snprintf(
        sqlcmd2,
        sizeof(sqlcmd2),
        "SELECT pg_database_size('logdb')"
    );

    threshold *= 1000000;  // converts to MB

    while (1) {
        nanos_day = get_day_nanos(buf);

        std::cout << iteration << ", " <<
            global_time_sec << ", " <<
            timeadvance << ", " <<
            nanos << ", " <<
            buf << ", " <<
            nanos_day << std::endl;

        res = logdb.ExecSQLtuples(sqlcmd1);
        if (PQntuples(res) < 0) {
            std::cerr << "failed to get db disk usage of ecommerce_db"
                << std::endl;
            return 1;
        }
        diskSpaceUsed = std::stoul(PQgetvalue(res, 0, 0));
        std::cout << "> ecommerce_db disk usage: " << diskSpaceUsed
                  << " bytes" << std::endl;
        if (diskSpaceUsed > threshold) {
            alert = ">> ALERT: logdb disk usage exceeds threshold ("
                      + std::to_string(diskSpaceUsed)
                      + " > " + std::to_string(threshold) + ") bytes";
            std::cout << alert << std::endl;

            log2db(
                logdb, "db-disk-usage-check",
                std::nullopt, std::nullopt, std::nullopt,
                "MONITOR", "monitor::db-disk-usage", "WARNING",
                pid, std::nullopt, std::make_optional(alert)
            );
        }

        res = logdb.ExecSQLtuples(sqlcmd2);
        if (PQntuples(res) < 0) {
            std::cerr << "failed to get db disk usage of logdb"
                << std::endl;
            return 1;
        }

        diskSpaceUsed = std::stoul(PQgetvalue(res, 0, 0));
        std::cout << "> logdb disk usage: " << diskSpaceUsed
                  << " bytes" << std::endl;
        if (diskSpaceUsed > threshold) {
            alert = ">> ALERT: logdb disk usage exceeds threshold ("
                      + std::to_string(diskSpaceUsed)
                      + " > " + std::to_string(threshold) + ") bytes";
            std::cout << alert << std::endl;

            log2db(
                logdb, "db-disk-usage-check",
                std::nullopt, std::nullopt, std::nullopt,
                "MONITOR", "monitor::db-disk-usage", "WARNING",
                pid, std::nullopt, std::make_optional(alert)
            );
        }

        std::cout << std::endl;

        // sleeps for 1 second
        micro_sleep(1000000);
        update_time();
        ++iteration;
    }
}
