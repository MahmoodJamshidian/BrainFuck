#include "core.hpp"

#ifndef BFX_CORE
#define BFX_CORE

void g_writeKey(char _val)
{
    std::cout << _val;
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
char g_readKey()
{
    return (char)_getch();
}

hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#else
char g_readKey()
{
    char c;
    system("stty raw");
    c = getchar();
    system("stty cooked");
    if (c > 0 && c < 32)
        std::cout << "\b\b  \b\b";
    else
        std::cout << "\b \b";
    return c;
}
#endif

template <typename... REST>
char *string_format(const char *__msg, const REST &...args)
{
    static char buffer[1024];
    snprintf(buffer, sizeof(buffer), __msg, args...);
    return buffer;
}

Traceback::Traceback(FILE *__stderr = stderr)
{
    this->__stderr = __stderr;
}
void Traceback::raise(Exceptions __exc = Exceptions::Exception, const char *__msg = "")
{
    if (this->throw_exc)
        throw std::runtime_error(string_format("%s: %s", exception_to_string(__exc), __msg));
    if (strcmp(__msg, "") != 0)
    {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        SetConsoleTextAttribute(hConsole, 4);
        std::cerr << exception_to_string(__exc) << ": " << __msg;
        SetConsoleTextAttribute(hConsole, 7);
        std::cerr << std::endl;
#else
        std::cerr << "\033[1;31m" << exception_to_string(__exc) << ": " << __msg << "\033[0m\n";
#endif
    }
    else
    {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        SetConsoleTextAttribute(hConsole, 4);
        std::cerr << exception_to_string(__exc);
        SetConsoleTextAttribute(hConsole, 7);
        std::cerr << std::endl;
#else
        std::cerr << "\033[1;31m" << exception_to_string(__exc) << "\033[0m\n";
#endif
    }
    exit(__exc);
}

void Traceback::raise(ExceptionStr __exc)
{
    this->raise(__exc.exception, __exc.msg);
}

Structure::Structure(react_func __reacter)
{
    this->__reacter = __reacter;
    structers.push_back(this);
}

void Structure::run(Environment *env, STR_DATA *str)
{
    this->__reacter(env, str);
}

Environment::Environment(std::vector<STR_DATA> structs) : structs(structs) {}

void Environment::add_signal(signal_func handler)
{
    sig_handlers.push_back(handler);
}

void Environment::signal(uint8_t sig)
{
    if(sig_handlers.size() > sig) sig_handlers.at(sig)();
}

void Environment::run()
{
    for (size_t _ind = 0; _ind < this->structs.size(); _ind++)
    {
        structs[_ind].type->run(this, &structs[_ind]);
    }
}

Structure S_ADD([](Environment *env, STR_DATA *str)
    {
        env->memory[env->pointers[env->selected_pointer]]++;
    }
);

Structure S_SUB([](Environment *env, STR_DATA *str)
    {
        env->memory[env->pointers[env->selected_pointer]]--;
    }
);

Structure S_LFT([](Environment *env, STR_DATA *str)
    {
        if(env->pointers[env->selected_pointer] <= 0)
        {
            __tb.raise(MemoryUnderflow, "out of range");
            return;
        }
        env->pointers[env->selected_pointer]--;
    }
);

Structure S_RGT([](Environment *env, STR_DATA *str)
    {
        env->pointers[env->selected_pointer]++;
        while(env->pointers[env->selected_pointer] >= env->memory.size())
        {
            env->memory.push_back(0);
        }
    }
);

Structure S_INP([](Environment *env, STR_DATA *str)
    {
        env->memory[env->pointers[env->selected_pointer]] = g_readKey();
    }
);

Structure S_OUT([](Environment *env, STR_DATA *str)
    {
        g_writeKey(env->memory[env->pointers[env->selected_pointer]]);
    }
);

Structure S_N_POINTER([](Environment *env, STR_DATA *str)
    {
        env->selected_pointer++;
        while(env->selected_pointer >= env->pointers.size())
        {
            env->pointers.push_back(env->pointers[env->selected_pointer-1]);
        }
    }
);

Structure S_P_POINTER([](Environment *env, STR_DATA *str)
    {
        if(env->selected_pointer <= 0)
        {
            __tb.raise(PointerUnderflow, "out of range");
            return;
        }
        env->selected_pointer--;
    }
);

Structure S_LOOP([](Environment *env, STR_DATA *str)
    {
        STR_DATA aloc;
        while(env->memory[env->pointers[env->selected_pointer]])
        {
            for(size_t i = 0; i < str->inner.size(); i++)
            {
                aloc = str->inner[i];
                str->inner[i].type->run(env, &aloc);
            }
        }
    }
);

Structure S_POINTER_LOOP([](Environment *env, STR_DATA *str)
    {
        STR_DATA aloc;
        while(env->pointers[env->selected_pointer])
        {
            for(size_t i = 0; i < str->inner.size(); i++)
            {
                aloc = str->inner[i];
                str->inner[i].type->run(env, &aloc);
            }
        }
    }
);

Structure S_RET([](Environment *env, STR_DATA *str)
    {
        env->signal(0);
    }
);

Structure S_PART([](Environment *env, STR_DATA *str)
    {
        Environment venv(str->inner);
        venv.memory = env->memory;
        venv.pointers = env->pointers;
        venv.selected_pointer = env->selected_pointer;
        venv.functions = env->functions;
        venv.add_signal([&]()
        {
            while(venv.pointers[venv.selected_pointer] >= env->memory.size())
                env->memory.push_back(0);
            env->memory[venv.pointers[venv.selected_pointer]] = venv.memory[venv.pointers[venv.selected_pointer]];
        });
        venv.run();
    }
);

Structure S_FUNC([](Environment *env, STR_DATA *str)
    {
        Environment func(str->inner);
        func.functions = env->functions;
        env->functions.push_back(func);
        env->memory[env->pointers[env->selected_pointer]] = env->functions.size() - 1;
    }
);

Structure S_CALL_FUNC([](Environment *env, STR_DATA *str)
    {
        if(env->memory[env->pointers[env->selected_pointer]] >= env->functions.size())
        {
            __tb.raise(Undefined, "function Undefined");
        }
        Environment inner(str->inner);
        inner.memory = env->memory;
        inner.pointers = env->pointers;
        inner.selected_pointer = env->selected_pointer;
        uint8_t cargs(0);
        inner.add_signal([&]()
            {
                env->functions[env->memory[env->pointers[env->selected_pointer]]].memory.push_back(inner.memory[inner.pointers[inner.selected_pointer]]);
                cargs++;
            }
        );
        inner.run();
        size_t creturn(0);
        env->functions[env->memory[env->pointers[env->selected_pointer]]].add_signal([&]()
            {
                ++creturn;
                while(env->pointers[env->selected_pointer]+creturn >= env->memory.size())
                {
                    env->memory.push_back(0);
                }
                env->memory.at(env->pointers[env->selected_pointer]+creturn) = env->functions[env->memory[env->pointers[env->selected_pointer]]].memory[env->functions[env->memory[env->pointers[env->selected_pointer]]].pointers[env->functions[env->memory[env->pointers[env->selected_pointer]]].selected_pointer]];
            }
        );
        env->functions[env->memory[env->pointers[env->selected_pointer]]].memory.at(0) = cargs;
        env->functions[env->memory[env->pointers[env->selected_pointer]]].run();
        env->functions[env->memory[env->pointers[env->selected_pointer]]].memory.clear();
        env->functions[env->memory[env->pointers[env->selected_pointer]]].pointers.clear();
    }
);
#endif