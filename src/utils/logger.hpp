#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>

#ifdef DEBUG_FLAG
#define LOG(message) std::cerr << "[LOG] " << message << std::endl
#else
#define LOG(message) do {} while(0)
#endif

#ifdef DEBUG_FLAG
#define ERROR(message) std::cerr << "[ERROR] " << message << std::endl
#else
#define ERROR(message) do {} while(0)
#endif

#endif // LOGGER_HPP