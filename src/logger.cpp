#include "logger.hpp"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

Logger::Logger() {
    logFile_.open("log.log", std::ios::out | std::ios::trunc);
    if (!logFile_.is_open()) {
        std::cerr << "Failed to open log file!" << std::endl;
    }
}

Logger::~Logger() {
    if (logFile_.is_open()) {
        logFile_.close();
    }
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex_);
    std::string logMessage = getCurrentTime() + " - " + message;

    if (logFile_.is_open()) {
        logFile_ << logMessage << std::endl;
    }

    std::cout << logMessage << std::endl;
}

void Logger::logDetailed(const std::string& action, const std::string& ip, int port, size_t dataSize) {
    std::lock_guard<std::mutex> lock(logMutex_);
    std::stringstream ss;
    ss << getCurrentTime() << " - "
       << action << " | IP: " << ip << " | Port: " << port
       << " | Data size: " << dataSize << " bytes";

    std::string logMessage = ss.str();

    if (logFile_.is_open()) {
        logFile_ << logMessage << std::endl;
    }

    std::cout << logMessage << std::endl;
}

std::string Logger::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&nowTime), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
