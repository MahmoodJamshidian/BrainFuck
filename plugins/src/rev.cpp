#ifdef BFX_BUILD
#ifndef BFX_CORE
#include "core.cpp"
#endif
#else
#include "libbfx.cpp"
#endif

Structure S_REV(
#ifndef BFX_BUILD
    "REV", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == '~')
        {
            return {__index, __index, &S_REV};
        }else{
            return {};
        }
    }, [](STR_DATA *str) -> std::string
    {
        return "{&S_REV}";
    }, 
#endif
    [](Environment *env, STR_DATA *str)
    {
        env->memory[env->pointers[env->selected_pointer]] = 255 - env->memory[env->pointers[env->selected_pointer]];
    }
);