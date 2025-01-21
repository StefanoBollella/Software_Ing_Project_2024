#ifndef utils_h
#define utils_h

#include<stdexcept>
#include<vector>
#include<string>
#include<sstream>
#include<cstring>

std::vector<ulong> strToULVector(const std::string &stringArr);

std::string vecToStr(std::vector<ulong> &carrierIDs);

bool isPositiveInteger(const char* value);

#endif // utils_h