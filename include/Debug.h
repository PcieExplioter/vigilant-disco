#pragma once
#include <iostream>

// Simple logging macros for standalone VisCheck
#define DEBUG_LOG(msg) do { std::cout << msg << std::endl; } while(0)
#define DEBUG_LOG_INFO(msg) do { std::cout << "[INFO] " << msg << std::endl; } while(0)
#define DEBUG_LOG_ERROR(msg) do { std::cout << "[ERROR] " << msg << std::endl; } while(0)
#define DEBUG_LOG_WARNING(msg) do { std::cout << "[WARNING] " << msg << std::endl; } while(0)

#define DEBUG_BUILD 1

