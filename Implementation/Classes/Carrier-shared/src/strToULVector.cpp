#include "utils.h"

/* Parses a string of the form "[x1, ..., xn]" into an unsigned long vector. */
std::vector<unsigned long> strToULVector(const std::string& stringArr) {
    std::vector<unsigned long> result;

    // Checks if the input starts with '[' and ends with ']'
    if (stringArr.front() != '[' || stringArr.back() != ']') {
        throw std::invalid_argument("Input string must be enclosed in square brackets");
    }

    // Removes the square brackets
    std::string numbersStr = stringArr.substr(1, stringArr.length() - 2);

    // Creates a stringstream from the string
    std::stringstream ss(numbersStr);
    std::string number;

    // Splits the string by commas
    while (std::getline(ss, number, ',')) {
        // Trim leading and trailing whitespace
        size_t start = number.find_first_not_of(" \t\n\r");
        size_t end = number.find_last_not_of(" \t\n\r");

        if (start == std::string::npos || end == std::string::npos) {
            throw std::invalid_argument("Invalid number format");
        }

        number = number.substr(start, end - start + 1);

        // Checks if number is negative
        if (number[0] == '-')
            throw std::domain_error("Numbers must be non-negative");

        // Converts the string to unsigned long
        try {
            unsigned long num = std::stoul(number);
            result.push_back(num);
        } catch (const std::exception& e) {
            throw std::invalid_argument("Invalid number format: " + number);
        }
    }

    return result;
}

