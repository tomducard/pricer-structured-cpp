#include "InputUtils.hpp"

#include <cctype>
#include <sstream>

namespace {
std::string trim(const std::string& input) {
    std::size_t first = 0;
    while (first < input.size() &&
           std::isspace(static_cast<unsigned char>(input[first]))) {
        ++first;
    }
    std::size_t last = input.size();
    while (last > first &&
           std::isspace(static_cast<unsigned char>(input[last - 1]))) {
        --last;
    }
    return input.substr(first, last - first);
}
} // namespace

std::string vectorToString(const std::vector<double>& values) {
    std::ostringstream oss;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            oss << ',';
        }
        oss << values[i];
    }
    return oss.str();
}

std::vector<double> parseTimesList(const std::string& text,
                                   const std::vector<double>& fallback) {
    // Parses comma-separated numbers into a vector; returns fallback if empty/invalid.
    if (text.empty()) {
        return fallback;
    }

    std::vector<double> result;
    std::stringstream ss(text);
    std::string token;
    while (std::getline(ss, token, ',')) {
        const std::string cleaned = trim(token);
        if (cleaned.empty()) {
            continue;
        }
        result.push_back(std::stod(cleaned));
    }

    return result.empty() ? fallback : result;
}
