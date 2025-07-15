#ifndef VERSION_H
#define VERSION_H

#define FILE_VERSION_MAJOR 2
#define FILE_VERSION_MINOR 0
#define FILE_VERSION_PATCH 9
#define FILE_VERSION_BUILD 2
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
std::string getVersionFromDateTime() {
    std::tm t = {};
    std::istringstream date_ss(__DATE__);
    date_ss >> std::get_time(&t, "%b %d %Y");
    std::istringstream time_ss(__TIME__);
    time_ss >> std::get_time(&t, "%H:%M:%S");
    std::ostringstream version_ss;
    version_ss << std::setfill('0') << std::setw(2) << (t.tm_year % 100)
               << std::setw(2) << (t.tm_mon + 1)
               << std::setw(2) << t.tm_mday
               << std::setw(2) << t.tm_hour
               << std::setw(2) << t.tm_min;
    return version_ss.str();
}

// Implementation of releaseType function
inline std::string releaseType() {
    return (FILE_VERSION_STABLE == 1) ? "Stable" : "PreRelease";
}

// Define RVO_COMPILATION as a macro to avoid multiple definition issues
#define RVO_COMPILATION getVersionFromDateTime()

#endif // __cplusplus

#endif // VERSION_H
