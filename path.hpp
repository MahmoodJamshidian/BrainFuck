#ifndef BFX_PATH
#define BFX_PATH
#if defined(_WIN32)
    #include <windows.h>
    #include <Shlwapi.h>
    #include <io.h> 

    #define access _access_s
#endif
#ifdef __APPLE__
    #include <libgen.h>
    #include <limits.h>
    #include <mach-o/dyld.h>
    #include <unistd.h>
#endif
#ifdef __linux__
    #include <limits.h>
    #include <libgen.h>
    #include <unistd.h>

    #if defined(__sun)
        #define PROC_SELF_EXE "/proc/self/path/a.out"
    #else
        #define PROC_SELF_EXE "/proc/self/exe"
    #endif

#endif

#include <string>
#include <string.h>

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