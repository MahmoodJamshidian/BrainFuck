#include "path.hpp"

#ifndef BFX_PATH
#define BFX_PATH
namespace path {
#if defined(_WIN32)
std::string getExecutablePath() {
   char rawPathName[MAX_PATH];
   GetModuleFileNameA(NULL, rawPathName, MAX_PATH);
   return std::string(rawPathName);
}
#endif
#ifdef __linux__
std::string getExecutablePath() {
   char rawPathName[PATH_MAX];
   realpath(PROC_SELF_EXE, rawPathName);
   return  std::string(rawPathName);
}
#endif
#ifdef __APPLE__
std::string getExecutablePath() {
    char rawPathName[PATH_MAX];
    char realPathName[PATH_MAX];
    uint32_t rawPathSize = (uint32_t)sizeof(rawPathName);
    if(!_NSGetExecutablePath(rawPathName, &rawPathSize)) {
        realpath(rawPathName, realPathName);
    }
    return  std::string(realPathName);
}
#endif
std::string getExecutableDir() {
    std::string res = getExecutablePath();
    return res.substr(0, res.find_last_of("/")+1);
}
}
#endif