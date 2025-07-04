#ifndef VERSION_H
#define VERSION_H

#define FILE_VERSION_MAJOR 2
#define FILE_VERSION_MINOR 0
#define FILE_VERSION_PATCH 7
#define FILE_VERSION_BUILD 5
#define FILE_VERSION_STABLE 0 // 1 for stable, 0 for prerelease

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define RVO_VERSION TOSTRING(FILE_VERSION_MAJOR) "." TOSTRING(FILE_VERSION_MINOR) "." TOSTRING(FILE_VERSION_PATCH) "." TOSTRING(FILE_VERSION_BUILD)

#ifdef __cplusplus
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

// Implementation of getVersionFromDateTime function
inline std::string getVersionFromDateTime() {
    // Get current time
    std::time_t t = std::time(nullptr);
    std::tm tm_ptr;

    // Use localtime_s for safer conversion
    localtime_s(&tm_ptr, &t);

    // Format as YYYYMMDDHHMM
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << (tm_ptr.tm_year % 100)
        << std::setw(2) << (tm_ptr.tm_mon + 1)
        << std::setw(2) << tm_ptr.tm_mday
        << std::setw(2) << tm_ptr.tm_hour
        << std::setw(2) << tm_ptr.tm_min;

    return oss.str();
}

// Implementation of releaseType function
inline std::string releaseType() {
    if (FILE_VERSION_STABLE == 1) {
        return "Stable";
    } else {
        return "PreRelease";
    }
}

// Define RVO_COMPILATION as a macro to avoid multiple definition issues
#define RVO_COMPILATION getVersionFromDateTime()

#endif // __cplusplus

#endif // VERSION_H
