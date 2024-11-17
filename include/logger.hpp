#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>
#include <mutex>
#include <string>

class Logger {
public:
    
    static Logger& getInstance();
    void logDetailed(const std::string& action, const std::string& ip, int port, size_t dataSize);
    void log(const std::string& message);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::ofstream logFile_;
    std::mutex logMutex_;
    std::string getCurrentTime();
};

#endif // LOGGER_HPP
