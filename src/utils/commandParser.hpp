#ifndef COMMAND_PARSER_HPP
#define COMMAND_PARSER_HPP

#include "stl/map.hpp"
#include <string>
#include <sstream>

using sjtu::map;

class CommandParser {
public:
  static bool parse(const std::string &line, int &timestamp,
                    std::string &command, map<char, std::string> &params);

  static bool validateParameters(const std::string &command,
                                 const map<char, std::string> &params);

private:
  // Helper functions for specific command validation
  static bool validateAddUser(const map<char, std::string> &params);
  static bool validateLogin(const map<char, std::string> &params);
  static bool validateLogout(const map<char, std::string> &params);
  static bool validateQueryProfile(const map<char, std::string> &params);
  static bool validateModifyProfile(const map<char, std::string> &params);
  static bool validateAddTrain(const map<char, std::string> &params);
  static bool validateDeleteTrain(const map<char, std::string> &params);
  static bool validateReleaseTrain(const map<char, std::string> &params);
  static bool validateQueryTrain(const map<char, std::string> &params);
  static bool validateQueryTicket(const map<char, std::string> &params);
  static bool validateQueryTransfer(const map<char, std::string> &params);
  static bool validateBuyTicket(const map<char, std::string> &params);
  static bool validateQueryOrder(const map<char, std::string> &params);
  static bool validateRefundTicket(const map<char, std::string> &params);
  static bool validateClean(const map<char, std::string> &params);
  static bool validateExit(const map<char, std::string> &params);
};

bool CommandParser::parse(const std::string &line, int &timestamp,
                          std::string &command,
                          map<char, std::string> &params) {
  params.clear();

  // Extract timestamp
  size_t timestamp_end = line.find(']');
  if (timestamp_end == std::string::npos || line[0] != '[') {
    return false;
  }

  try {
    timestamp = stoi(line.substr(1, timestamp_end - 1));
  } catch (...) {
    return false;
  }

  // Extract command and parameters
  size_t cmd_start = timestamp_end + 2; // Skip "] "
  if (cmd_start >= line.length()) {
    return false;
  }

  size_t cmd_end = line.find(' ', cmd_start);
  if (cmd_end == std::string::npos) {
    command = line.substr(cmd_start);
    return true; // Command with no parameters
  }

  command = line.substr(cmd_start, cmd_end - cmd_start);

  // Parse parameters
  std::string param_str = line.substr(cmd_end + 1);
  std::istringstream iss(param_str);
  std::string token;

  while (iss >> token) {
    if (token[0] != '-' || token.length() < 2) {
      return false; // Invalid parameter format
    }

    char key = token[1];
    std::string value;

    if (iss.peek() != EOF && iss.peek() != '-') {
      iss >> value;
      // Trim leading space
      value.erase(0, value.find_first_not_of(' '));
    }

    params[key] = value;
  }

  return validateParameters(command, params);
}

bool CommandParser::validateParameters(const std::string &command,
                                       const map<char, std::string> &params) {
  if (command == "add_user")
    return validateAddUser(params);
  if (command == "login")
    return validateLogin(params);
  if (command == "logout")
    return validateLogout(params);
  if (command == "query_profile")
    return validateQueryProfile(params);
  if (command == "modify_profile")
    return validateModifyProfile(params);
  if (command == "add_train")
    return validateAddTrain(params);
  if (command == "delete_train")
    return validateDeleteTrain(params);
  if (command == "release_train")
    return validateReleaseTrain(params);
  if (command == "query_train")
    return validateQueryTrain(params);
  if (command == "query_ticket")
    return validateQueryTicket(params);
  if (command == "query_transfer")
    return validateQueryTransfer(params);
  if (command == "buy_ticket")
    return validateBuyTicket(params);
  if (command == "query_order")
    return validateQueryOrder(params);
  if (command == "refund_ticket")
    return validateRefundTicket(params);
  if (command == "clean")
    return validateClean(params);
  if (command == "exit")
    return validateExit(params);

  return false; // Unknown command
}

// Validation functions for each command
bool CommandParser::validateAddUser(const map<char, std::string> &params) {
  // Required: -c -u -p -n -m -g
  if (params.count('c') == 0 || params.count('u') == 0 ||
      params.count('p') == 0 || params.count('n') == 0 ||
      params.count('m') == 0 || params.count('g') == 0) {
    return false;
  }

  // Validate privilege is a number between 0-10
  try {
    int privilege = stoi(params.at('g'));
    if (privilege < 0 || privilege > 10)
      return false;
  } catch (...) {
    return false;
  }

  return true;
}

bool CommandParser::validateLogin(const map<char, std::string> &params) {
  // Required: -u -p
  return params.count('u') && params.count('p');
}

bool CommandParser::validateLogout(const map<char, std::string> &params) {
  // Required: -u
  return params.count('u');
}

bool CommandParser::validateQueryProfile(const map<char, std::string> &params) {
  // Required: -c -u
  return params.count('c') && params.count('u');
}

bool CommandParser::validateModifyProfile(
    const map<char, std::string> &params) {
  // Required: -c -u
  // Optional: -p -n -m -g
  if (!params.count('c') || !params.count('u')) {
    return false;
  }

  // If privilege is provided, validate it
  if (params.count('g')) {
    try {
      int privilege = stoi(params.at('g'));
      if (privilege < 0 || privilege > 10)
        return false;
    } catch (...) {
      return false;
    }
  }

  return true;
}

bool CommandParser::validateAddTrain(const map<char, std::string> &params) {
  // Required: -i -n -m -s -p -x -t -o -d -y
  if (params.count('i') == 0 || params.count('n') == 0 ||
      params.count('m') == 0 || params.count('s') == 0 ||
      params.count('p') == 0 || params.count('x') == 0 ||
      params.count('t') == 0 || params.count('o') == 0 ||
      params.count('d') == 0 || params.count('y') == 0) {
    return false;
  }

  // Validate stationNum is a number between 2-100
  try {
    int stationNum = stoi(params.at('n'));
    if (stationNum < 2 || stationNum > 100)
      return false;
  } catch (...) {
    return false;
  }

  // Validate seatNum is a positive number
  try {
    int seatNum = stoi(params.at('m'));
    if (seatNum <= 0)
      return false;
  } catch (...) {
    return false;
  }

  // Validate type is a single uppercase letter
  if (params.at('y').length() != 1 || !isupper(params.at('y')[0])) {
    return false;
  }

  return true;
}

bool CommandParser::validateDeleteTrain(const map<char, std::string> &params) {
  // Required: -i
  return params.count('i');
}

bool CommandParser::validateReleaseTrain(const map<char, std::string> &params) {
  // Required: -i
  return params.count('i');
}

bool CommandParser::validateQueryTrain(const map<char, std::string> &params) {
  // Required: -i -d
  return params.count('i') && params.count('d');
}

bool CommandParser::validateQueryTicket(const map<char, std::string> &params) {
  // Required: -s -t -d
  // Optional: -p (default: time)
  if (!params.count('s') || !params.count('t') || !params.count('d')) {
    return false;
  }

  // Validate -p if present
  if (params.count('p') && params.at('p') != "time" &&
      params.at('p') != "cost") {
    return false;
  }

  return true;
}

bool CommandParser::validateQueryTransfer(
    const map<char, std::string> &params) {
  // Same as query_ticket
  return validateQueryTicket(params);
}

bool CommandParser::validateBuyTicket(const map<char, std::string> &params) {
  // Required: -u -i -d -n -f -t
  // Optional: -q (default: false)
  if (params.count('u') == 0 || params.count('i') == 0 ||
      params.count('d') == 0 || params.count('n') == 0 ||
      params.count('f') == 0 || params.count('t') == 0) {
    return false;
  }

  // Validate ticket number is positive
  try {
    int num = stoi(params.at('n'));
    if (num <= 0)
      return false;
  } catch (...) {
    return false;
  }

  // Validate -q if present
  if (params.count('q') && params.at('q') != "true" &&
      params.at('q') != "false") {
    return false;
  }

  return true;
}

bool CommandParser::validateQueryOrder(const map<char, std::string> &params) {
  // Required: -u
  return params.count('u');
}

bool CommandParser::validateRefundTicket(const map<char, std::string> &params) {
  // Required: -u
  // Optional: -n (default: 1)
  if (!params.count('u')) {
    return false;
  }

  // Validate -n if present
  if (params.count('n')) {
    try {
      int n = stoi(params.at('n'));
      if (n <= 0)
        return false;
    } catch (...) {
      return false;
    }
  }

  return true;
}

bool CommandParser::validateClean(const map<char, std::string> &params) {
  // No parameters expected
  return params.empty();
}

bool CommandParser::validateExit(const map<char, std::string> &params) {
  // No parameters expected
  return params.empty();
}

#endif // COMMAND_PARSER_HPP