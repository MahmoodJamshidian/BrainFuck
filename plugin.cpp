#include "libbfx.hpp"
#include <iostream>

Structure S_DLTA("DLTA", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == '^')
        {
            return {__index, __index, get_structure("DLTA")};
        }else{
            return {};
        }
    }, [](Environment *env, STR_DATA *str)
    {
        std::cout << "/dev/rag0&^%$#@!~";

    }, [](STR_DATA *str) -> std::string
    {
        return "{&S_DLTA}";
    }
);