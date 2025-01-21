#include "utils.h"

std::string vecToStr(std::vector<ulong> &customerIDs) {
    std::string temp{"["}, result;

    for (ulong id: customerIDs) {
        temp += std::to_string(id) + ",";
    }
    result = temp.substr(0, temp.length() - 1);
    result += "]";
    return result; 
}
