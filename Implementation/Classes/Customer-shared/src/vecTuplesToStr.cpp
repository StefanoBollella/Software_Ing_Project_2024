#include "utils.h"

std::string vecTuplesToStr(
    std::vector<std::pair<ulong, ulong>> &products
) {
    std::string temp{"["}, result;

    for (std::pair<ulong, ulong> p: products) {
       temp += "(" + std::to_string(p.first) + "," 
           + std::to_string(p.second) + "), "; 
    }
    result = temp.substr(0, temp.length() - 2);
    result += "]";
    return result;
}
