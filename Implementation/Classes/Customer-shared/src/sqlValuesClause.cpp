#include "utils.h"


void sqlValuesClause(
        char *buffer, size_t buffSize,
        std::vector<std::string> tuples
) {

    std::string funcName = __func__;

    if (buffer == nullptr) {
        std::invalid_argument(
            funcName + ": expected 'buffer' pointer, got nullptr instead"
        );
    }
    if (buffSize == 0)
        std::invalid_argument(funcName + ": expected 'buffSize' > 0");

    size_t offset = 0;
    int nRows, i, j;
    nRows = tuples.size();

    // VALUES (...), (...), (...), ...
    offset += snprintf(buffer, buffSize, "VALUES ");
    for (i = 0; i < nRows; ++i) {
        offset += snprintf(buffer + offset, buffSize - offset, "%s", tuples[i].c_str());
        // dont' put comma after the last tuple
        if (i < nRows - 1)
            offset += snprintf(buffer + offset, buffSize - offset, ", ");
    }
}
