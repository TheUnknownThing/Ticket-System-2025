#include "services/orderManager.hpp"
#include "services/trainManager.hpp"
#include "services/userManager.hpp"
#include "stl/map.hpp"
#include "utils/commandParser.hpp"
#include <iostream>

using sjtu::map;

//#define DEBUG_FLAG

#ifdef DEBUG_FLAG
#define DEBUG_CMD(cmd) std::cout << "[DEBUG] Command: " << (cmd) << std::endl;
#else
#define DEBUG_CMD(cmd)
#endif


int main() {
  UserManager userManager("users");
  // TrainManager trainManager;
  // OrderManager orderManager(&userManager, &trainManager);

  std::string line;
  while (getline(std::cin, line)) {
    int timestamp;
    std::string command;
    map<char, std::string> params;

    if (!CommandParser::parse(line, timestamp, command, params)) {
      std::cout << "[" << timestamp << "] -1"; // Invalid command
      continue;
    }

    DEBUG_CMD(command);

    std::cout << "[" << timestamp << "] ";

    try {
      if (command == "add_user") {
        std::cout << (userManager.addUser(params['c'], params['u'], params['p'],
                                         params['n'], params['m'],
                                         std::stoi(params['g'])) ? "0" : "-1");
      } else if (command == "login") {
        std::cout << (userManager.login(params['u'], params['p']) ? "0" : "-1") 
                 ;
      } else if (command == "logout") {
        std::cout << (userManager.logout(params['u']) ? "0" : "-1");
      } else if (command == "query_profile") {
        std::cout << userManager.queryProfile(params['c'], params['u']);
      } else if (command == "modify_profile") {
        std::cout << userManager.modifyProfile(
            params['c'], params['u'], params.count('p') ? params['p'] : "",
            params.count('n') ? params['n'] : "",
            params.count('m') ? params['m'] : "",
            params.count('g') ? std::stoi(params['g']) : -1);
      } /*else if (command == "add_train") {
        std::cout << trainManager.addTrain(
            params['i'], std::stoi(params['n']), std::stoi(params['m']),
            params['s'], params['p'], params['x'], params['t'], params['o'],
            params['d'], params['y']);
      } else if (command == "delete_train") {
        std::cout << trainManager.deleteTrain(params['i']);
      } else if (command == "release_train") {
        std::cout << trainManager.releaseTrain(params['i']);
      } else if (command == "query_train") {
        std::string result = trainManager.queryTrain(params['i'], params['d']);
        std::cout << (result.empty() ? "-1" : result);
      } else if (command == "query_ticket") {
        std::string sortBy = params.count('p') ? params['p'] : "time";
        std::string result = trainManager.queryTicket(params['s'], params['t'],
                                                      params['d'], sortBy);
        std::cout << (result.empty() ? "0" : result);
      } else if (command == "query_transfer") {
        std::string sortBy = params.count('p') ? params['p'] : "time";
        std::string result = trainManager.queryTransfer(
            params['s'], params['t'], params['d'], sortBy);
        std::cout << (result.empty() ? "0" : result);
      } else if (command == "buy_ticket") {
        bool queue = params.count('q') && params['q'] == "true";
        std::string result = orderManager.buyTicket(
            params['u'], params['i'], params['d'], std::stoi(params['n']),
            params['f'], params['t'], queue);
        std::cout << result;
      } else if (command == "query_order") {
        std::string result = orderManager.queryOrder(params['u']);
        std::cout << (result.empty() ? "-1" : result);
      } else if (command == "refund_ticket") {
        int n = params.count('n') ? std::stoi(params['n']) : 1;
        std::cout << orderManager.refundTicket(params['u'], n);
      }*/ else if (command == "clean") {
        userManager.clean();
        // trainManager.clean();
        // orderManager.clean();
        std::cout << "0";
      } else if (command == "exit") {
        userManager.clearLoggedInUsers();
        std::cout << "bye";
        break;
      } else {
        std::cout << "-1"; // Unknown command
      }
    } catch (const std::exception &e) {
      std::cout << "-1"; // Exception occurred
    }

    std::cout  << "\n";
  }
  return 0;
}