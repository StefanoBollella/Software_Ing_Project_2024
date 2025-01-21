#include "utils.h"
#include <regex>

std::vector<std::pair<ulong, long>> strTuplesToVec(
    const std::string &stringTuples
) {
    // Checks if the input starts with '[' and ends with ']'
    if (stringTuples.front() != '[' || stringTuples.back() != ']') {
        throw std::invalid_argument(
            "Input string must be enclosed in square brackets"
        );
    }

    std::vector<std::pair<ulong, long>> res;
    std::string tuples;
    unsigned long x;
    long y;

    tuples = stringTuples;

    std::regex tupleRegex("[0-9]+");
    for (std::smatch tupleMatch;
         regex_search(tuples, tupleMatch, tupleRegex);) {
        x = std::stoul(tupleMatch.str());
        // updates the tuples to the suffix after the match
        tuples = tupleMatch.suffix();

        // reads the second component value
        regex_search(tuples, tupleMatch, tupleRegex);
        y = std::stoul(tupleMatch.str());
        tuples = tupleMatch.suffix();

        res.emplace_back(std::make_pair(x, y));
    }

    return res;
}

