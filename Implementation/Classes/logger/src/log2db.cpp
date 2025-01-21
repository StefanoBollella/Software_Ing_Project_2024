#include "log2db.h"
#include <cassert>

#define SQLCMD_BUFF_SIZE 5000

void log2db(
    Con2DB db, std::string opType,
    std::optional<std::string> usrID,
    std::optional<std::string> usrState,
    std::optional<std::string> reqID,
    std::string component, std::string srcContext,
    std::string logLvl, unsigned pid,
    std::optional<unsigned long> durationNanos,
    std::optional<std::string> logInfo
) {
    char sqlcmd[SQLCMD_BUFF_SIZE];
    PGresult *res;
    unsigned op_id;

    snprintf(sqlcmd, sizeof(sqlcmd), "BEGIN");
    db.ExecSQLcmd(sqlcmd);
    memset(sqlcmd, '\0', sizeof(sqlcmd));


    // \'%s\' makes it so that the string value
    // is escaped and treated as a string, instead
    // of being converted implicitly by the
    // attribute type
    snprintf(
        sqlcmd, sizeof(sqlcmd),
        "INSERT INTO operation(op_type, user_id, "
        "user_state, req_id, component, source_context) "
        "VALUES "
        "(\'%s\', %s, %s, %s, \'%s\', \'%s\') "
        "RETURNING id",
        opType.c_str(),
        usrID.value_or("NULL").c_str(),
        usrState.value_or("NULL").c_str(),
        reqID.value_or("NULL").c_str(),
        component.c_str(),
        srcContext.c_str()
    );
    res = db.ExecSQLtuples(sqlcmd);
    assert(PQntuples(res) > 0);
    op_id = std::stoul(PQgetvalue(res, 0, PQfnumber(res, "id")));
    memset(sqlcmd, '\0', sizeof(sqlcmd));

    snprintf(
        sqlcmd, sizeof(sqlcmd),
        "INSERT INTO log_table(level, pid, "
        "operation_id, nanosec, duration_nanos, loginfo) "
        "VALUES "
        "(\'%s\', %u, %u, %lu,  %lu, \'%s\')",
        logLvl.c_str(), pid, op_id, nanos_day,
        durationNanos.value_or(0),
        logInfo.value_or("NULL").c_str()
    );
    db.ExecSQLcmd(sqlcmd);
    memset(sqlcmd, '\0', sizeof(sqlcmd));

    snprintf(sqlcmd, sizeof(sqlcmd), "COMMIT");
    db.ExecSQLcmd(sqlcmd);
}
