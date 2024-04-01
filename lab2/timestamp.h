#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <iomanip>

class Timestamp {
public:
    static std::string getCurrentTimeAsString() {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        struct tm* ptm = std::localtime(&time);
        std::ostringstream oss;
        oss << "[" << std::put_time(ptm, "%T") << "] ";
        return oss.str();
    }
};

#endif
