#ifndef UTILS_H
#define UTILS_H

#include <stdexcept>
#include <vector>
#include <string>
#include <sstream>

std::vector<ulong> strToULVector(const std::string &stringArr);

std::string vecToStr(std::vector<ulong> &customerIDs);

std::string vecTuplesToStr(
    std::vector<std::pair<ulong, ulong>> &products
);

std::vector<std::pair<ulong, long>> strTuplesToVec(
    const std::string &stringTuples
);

void sqlValuesClause(
        char *buffer, size_t buffSize,
        std::vector<std::string> tuples
);

#endif  // UTILS_H
