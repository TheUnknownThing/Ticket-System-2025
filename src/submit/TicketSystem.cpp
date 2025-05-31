//#define DEBUG_FLAG
#include "services/orderManager.hpp"
#include "services/trainManager.hpp"
#include "services/userManager.hpp"
#include "stl/map.hpp"
#include "utils/commandParser.hpp"
#include "utils/logger.hpp"
#include <iostream>

using sjtu::map;

int main() {
  //freopen("../test/TicketSystem/21.in", "r", stdin);
  freopen("cerr.log", "w", stderr);

  UserManager userManager("users");
  TrainManager trainManager("trains");
  OrderManager orderManager("orders", &trainManager);

  std::string line;
  while (getline(std::cin, line)) {
    int timestamp;
    std::string command;
    map<char, std::string> params;

    if (!CommandParser::parse(line, timestamp, command, params)) {
      LOG("Invalid command received: " + line);
      std::cout << "[" << timestamp << "] -1"; // Invalid command
      continue;
    }

    LOG("Processing command: " + command +
        " at timestamp: " + std::to_string(timestamp));

    std::cout << "[" << timestamp << "] ";

    try {
      if (command == "add_user") {
        bool result = userManager.addUser(params['c'], params['u'], params['p'],
                                          params['n'], params['m'],
                                          std::stoi(params['g']));
        LOG("add_user operation for user '" + params['u'] +
            "' result: " + (result ? "success" : "failed"));
        std::cout << (result ? "0" : "-1");
      } else if (command == "login") {
        bool result = userManager.login(params['u'], params['p']);
        LOG("login operation for user '" + params['u'] +
            "' result: " + (result ? "success" : "failed"));
        std::cout << (result ? "0" : "-1");
      } else if (command == "logout") {
        bool result = userManager.logout(params['u']);
        LOG("logout operation for user '" + params['u'] +
            "' result: " + (result ? "success" : "failed"));
        std::cout << (result ? "0" : "-1");
      } else if (command == "query_profile") {
        auto result = userManager.queryProfile(params['c'], params['u']);
        LOG("query_profile operation for user '" + params['u'] + "' by '" +
            params['c'] + "'");
        std::cout << result;
      } else if (command == "modify_profile") {
        auto result = userManager.modifyProfile(
            params['c'], params['u'], params.count('p') ? params['p'] : "",
            params.count('n') ? params['n'] : "",
            params.count('m') ? params['m'] : "",
            params.count('g') ? std::stoi(params['g']) : -1);
        LOG("modify_profile operation for user '" + params['u'] + "' by '" +
            params['c'] + "'");
        std::cout << result;
      } else if (command == "add_train") {
        auto result = trainManager.addTrain(
            params['i'], std::stoi(params['n']), std::stoi(params['m']),
            params['s'], params['p'], params['x'], params['t'], params['o'],
            params['d'], params['y'][0]);
        LOG("add_train operation for train '" + params['i'] +
            "' result: " + std::to_string(result));
        std::cout << result;
      } else if (command == "delete_train") {
        auto result = trainManager.deleteTrain(params['i']);
        LOG("delete_train operation for train '" + params['i'] +
            "' result: " + std::to_string(result));
        std::cout << result;
      } else if (command == "release_train") {
        auto result = trainManager.releaseTrain(params['i']);
        LOG("release_train operation for train '" + params['i'] +
            "' result: " + std::to_string(result));
        std::cout << result;
      } else if (command == "query_train") {
        auto result = trainManager.queryTrain(params['i'], params['d']);
        LOG("query_train operation for train '" + params['i'] + "' on date '" +
            params['d'] + "'");
        std::cout << (result.empty() ? "-1" : result);
      } else if (command == "query_ticket") {
        std::string sortBy = params.count('p') ? params['p'] : "time";
        auto result = trainManager.queryTicket(params['s'], params['t'],
                                               params['d'], sortBy);
        LOG("query_ticket operation from '" + params['s'] + "' to '" +
            params['t'] + "' on '" + params['d'] + "' sorted by " + sortBy);
        std::cout << (result.empty() ? "0" : result);
      } else if (command == "query_transfer") {
        std::string sortBy = params.count('p') ? params['p'] : "time";
        auto result = trainManager.queryTransfer(params['s'], params['t'],
                                                 params['d'], sortBy);
        LOG("query_transfer operation from '" + params['s'] + "' to '" +
            params['t'] + "' on '" + params['d'] + "' sorted by " + sortBy);
        std::cout << (result.empty() ? "0" : result);
      } else if (command == "buy_ticket") {
        bool queue = params.count('q') && params['q'] == "true";
        if (userManager.isLoggedIn(params['u']) == false) {
          ERROR("buy_ticket failed: user '" + params['u'] + "' not logged in");
          std::cout << "-1"; // User not logged in
        } else {
          auto result = orderManager.buyTicket(
              params['u'], params['i'], params['d'], std::stoi(params['n']),
              params['f'], params['t'], queue, timestamp);
          LOG("buy_ticket operation for user '" + params['u'] + "' train '" +
              params['i'] + "' " + params['n'] + " tickets from '" +
              params['f'] + "' to '" + params['t'] +
              "' queue: " + (queue ? "true" : "false"));
          if (result == 0) {
            std::cout << "queue";
          } else {
            std::cout << result;
          }
        }
      } else if (command == "query_order") {
        if (!userManager.isLoggedIn(params['u'])) {
          ERROR("query_order failed: user '" + params['u'] + "' not logged in");
          std::cout << "-1";
        } else {
          auto result = orderManager.queryOrder(params['u']);
          LOG("query_order operation for user '" + params['u'] + "' returned " +
              std::to_string(result.size()) + " orders");
          if (result.empty()) {
            std::cout << "0";
          } else {
            std::cout << result.size() << "\n";
            for (int i = result.size() - 1; i >= 0; --i) {
              std::cout << result[i];
              if (i > 0) {
                std::cout << "\n";
              }
            }
          }
        }
      } else if (command == "refund_ticket") {
        int n = params.count('n') ? std::stoi(params['n']) : 1;
        if (!userManager.isLoggedIn(params['u'])) {
          ERROR("refund_ticket failed: user '" + params['u'] +
                "' not logged in");
          std::cout << "-1";
        } else {
          bool result = orderManager.refundTicket(params['u'], n);
          LOG("refund_ticket operation for user '" + params['u'] +
              "' ticket #" + std::to_string(n) +
              " result: " + (result ? "success" : "failed"));
          std::cout << (result ? "0" : "-1");
        }
      } else if (command == "clean") {
        LOG("clean operation started");
        userManager.clean();
        // trainManager.clean();
        // orderManager.clean();
        LOG("clean operation completed");
        std::cout << "0";
      } else if (command == "exit") {
        LOG("exit command received, clearing logged in users");
        userManager.clearLoggedInUsers();
        LOG("System shutdown");
        std::cout << "bye";
        break;
      } else {
        ERROR("Unknown command received: " + command);
        std::cout << "-1"; // Unknown command
      }
    } catch (const std::exception &e) {
      ERROR("Exception occurred while processing command '" + command +
            "': " + e.what());
      std::cout << "-1"; // Exception occurred
    }
    std::cout << "\n";
  }
  return 0;
}