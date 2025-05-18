#include <iostream>
#include "utils/commandParser.hpp"
#include "services/userManager.hpp"
#include "services/orderManager.hpp"

int main() {
    std::string line;
    while (getline(std::cin, line)) {
        int timestamp;
        std::string command;
        map<char, std::string> params;
        
        if (!CommandParser::parse(line, timestamp, command, params)) {
            std::cout << "[" << timestamp << "] -1" << std::endl;
            continue;
        }
        
        std::cout << "[" << timestamp << "] ";
        if (command == "add_user") {
            std::cout << "0" << std::endl;
        } else if (command == "exit") {
            std::cout << "bye" << std::endl;
            break;
        } else {
            std::cout << "0" << std::endl;
        }
    }
    return 0;
}