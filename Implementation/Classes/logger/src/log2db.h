#ifndef LOG2DB_H
#define LOG2DB_H

#include <string>
#include <cstring>
#include <optional>

#include "../../Clock/src/clock.h"

// POSTGRESQL LIBS
#include "../../con2db/pgsql.h"

void log2db(
    Con2DB db,
    std::string reqType,
    std::optional<std::string> usrID,
    std::optional<std::string> usrState,
    std::optional<std::string> reqID,
    std::string component,
    std::string srcContext,
    std::string logLvl, unsigned pid,
    std::optional<unsigned long> durationNanos,
    std::optional<std::string> logInfo
);

#endif
