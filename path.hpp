#include <string>
#include <string.h>

#ifndef BFX_PATH_HEADER
#define BFX_PATH_HEADER
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

std::string getExecutablePath();
std::string getExecutableDir();
#endif