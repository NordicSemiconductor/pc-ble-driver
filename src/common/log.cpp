#include "log.h"

#include <iomanip>

typedef void(*logCallbackFunction) (const char *message);

logCallbackFunction logCallback = nullptr;
std::mutex logMutex;

void log(const std::chrono::time_point<std::chrono::high_resolution_clock> timestamp,
        const std::thread::id threadID,
        const std::string &message)
{
    if (logCallback == nullptr) {
        return;
    }

    std::stringstream stream;

    stream << "@" << std::right << std::setfill('0') << std::setw(10) << std::chrono::duration_cast<std::chrono::microseconds>(timestamp.time_since_epoch()).count()
         << " [" << std::setw(5) << threadID << "] "
         << "| " << message << std::endl << std::flush;

    logCallback(stream.str().c_str());
}

void setLogCallback(logCallbackFunction callback)
{
    std::unique_lock<std::mutex> lock (logMutex);
    logCallback = callback;
}
