#include "utils.h"

std::string vecToStr(std::vector<ulong> &carrierIDs) {
    std::string temp{"["}, result;

    for (ulong id: carrierIDs) {
        temp += std::to_string(id) + ",";
    }
    result = temp.substr(0, temp.length() - 1);
    result += "]";
    return result; 
}
