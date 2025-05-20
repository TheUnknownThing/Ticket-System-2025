#ifndef SPLIT_STRING_HPP
#define SPLIT_STRING_HPP

#include "stl/vector.hpp"
#include "utils/string32.hpp"
#include <sstream>
#include <string>

using sjtu::string32;
using sjtu::vector;

vector<string32> splitString(const std::string &s, char delimiter) {
  vector<string32> tokens;
  if (s.empty())
    return tokens;
  std::string token_std;
  std::istringstream tokenStream(s);
  while (std::getline(tokenStream, token_std, delimiter)) {
    tokens.push_back(string32(token_std.c_str()));
  }
  return tokens;
}

// Helper to split a string by a delimiter into int
vector<int> splitStringToInt(const std::string &s, char delimiter) {
  vector<int> tokens;
  if (s.empty())
    return tokens;
  if (s == "_")
    return tokens; // Handle special case for stopoverTimes

  std::string token_std;
  std::istringstream tokenStream(s);
  while (std::getline(tokenStream, token_std, delimiter)) {
    if (!token_std.empty()) {
      tokens.push_back(std::stoi(token_std));
    }
  }
  return tokens;
}

// Helper to split std::string by a delimiter into std::string
vector<std::string> splitStdString(const std::string &s, char delimiter) {
  vector<std::string> tokens;
  if (s.empty())
    return tokens;
  std::string token_std;
  std::istringstream tokenStream(s);
  while (std::getline(tokenStream, token_std, delimiter)) {
    tokens.push_back(token_std);
  }
  return tokens;
}

#endif // SPLIT_STRING_HPP