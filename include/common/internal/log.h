#ifndef LOG_H__
#define LOG_H__

#include <thread>
#include <chrono>
#include <sstream>
#include <mutex>
#include <vector>
#include <iomanip>

typedef void(*logCallbackFunction) (const char *message);

extern std::mutex logMutex;

#define DEBUG(message) do { \
    std::unique_lock<std::mutex> lock (logMutex); \
    std::stringstream stream; \
    stream << message; \
    log(std::chrono::high_resolution_clock::now(), \
        std::this_thread::get_id(), \
        stream.str()); \
} while(0);

void log(const std::chrono::time_point<std::chrono::high_resolution_clock> timestamp,
        const std::thread::id threadID,
        const std::string &message);

void setLogCallback(logCallbackFunction callback);

#endif // LOG_H__
