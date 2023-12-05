#include "path.hpp"

#ifndef BFX_PATH
#define BFX_PATH

namespace path
{
    fs::path getExecutablePath()
    {
#if defined(_WIN32)
    char rawPathName[MAX_PATH];
    GetModuleFileNameA(NULL, rawPathName, MAX_PATH);
    return fs::path(rawPathName);
#endif
#ifdef __linux__
    char rawPathName[PATH_MAX];
    realpath(PROC_SELF_EXE, rawPathName);
    return  fs::path(rawPathName);
#endif
#ifdef __APPLE__
        char rawPathName[PATH_MAX];
        char realPathName[PATH_MAX];
        uint32_t rawPathSize = (uint32_t)sizeof(rawPathName);
        if(!_NSGetExecutablePath(rawPathName, &rawPathSize)) {
            realpath(rawPathName, realPathName);
        }
        return  fs::path(realPathName);
#endif
    }

    fs::path getExecutableDir()
    {
        return getExecutablePath().parent_path();
    }
}
#endif