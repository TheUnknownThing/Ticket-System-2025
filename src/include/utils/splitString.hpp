#ifndef SPLIT_STRING_HPP
#define SPLIT_STRING_HPP

#include <string>
#include <cstdio>
#include <sstream>
#include "stl/vector.hpp"
#include "utils/string32.hpp"

using sjtu::vector;
using sjtu::string32;

vector<string32> splitS32String(const string32& s, char delimiter) {
    vector<string32> tokens;
    if (s.empty()) return tokens;
    std::string s_std = s.toString();
    std::string token_std;
    std::istringstream tokenStream(s_std);
    while (std::getline(tokenStream, token_std, delimiter)) {
        tokens.push_back(string32(token_std.c_str()));
    }
    return tokens;
}

// Helper to split a string32 by a delimiter into int
vector<int> splitS32StringToInt(const string32& s, char delimiter) {
    vector<int> tokens;
    if (s.empty()) return tokens;
    std::string s_std = s.toString();
    if (s_std == "_") return tokens; // Handle special case for stopoverTimes

    std::string token_std;
    std::istringstream tokenStream(s_std);
    while (std::getline(tokenStream, token_std, delimiter)) {
        if (!token_std.empty()) {
            try {
                tokens.push_back(std::stoi(token_std));
            } catch (const std::exception&) {
                // Error during conversion, could add error logging or throw
                // For now, may result in an empty or partially filled vector
            }
        }
    }
    return tokens;
}
// Helper to split std::string by a delimiter into std::string
vector<std::string> splitStdString(const std::string& s, char delimiter) {
    vector<std::string> tokens;
    if (s.empty()) return tokens;
    std::string token_std;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token_std, delimiter)) {
        tokens.push_back(token_std);
    }
    return tokens;
}

#endif // SPLIT_STRING_HPP