#include <iostream>
#include <string.h>
#include <vector>
#include <functional>

#define CORE

#define INITIAL_REGISTRY_ARGS 2

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <conio.h>
char readKey()
{
    return (char)_getch();
}
void clear()
{
    system("cls");
}
#else
char readKey()
{
    char c;
    system("stty raw");
    c = getchar();
    system("stty cooked");
    std::cout << "\b \b";
    return c;
}
void clear()
{
    system("clear");
}
#endif

enum Exceptions
{
    Exception = 1,
    MemoryOverflow,
    MemoryUnderflow,
    PointerOverflow,
    PointerUnderflow,
    Undefined
};

constexpr const char *exception_to_string(Exceptions e) throw()
{
    switch (e)
    {
    case Exceptions::MemoryOverflow:
        return "MemoryOverflow";
    case Exceptions::MemoryUnderflow:
        return "MemoryUnderflow";
    case Exceptions::PointerOverflow:
        return "PointerOverflow";
    case Exceptions::PointerUnderflow:
        return "PointerUnderflow";
    case Exceptions::Undefined:
        return "Undefined";
    case Exceptions::Exception:
        return "Exception";
    default:
        throw std::invalid_argument("Unimplemented Exception");
    }
}

struct ExceptionStr
{
    Exceptions exception;
    char *msg;
};

class Traceback
{
    FILE *__stderr;

public:
    Traceback(FILE *__stderr = stderr)
    {
        this->__stderr = __stderr;
    }
    void raise(Exceptions __exc = Exception, const char *__msg = "")
    {
        if (strcmp(__msg, "") != 0)
        {
            std::cerr << "\033[1;31m" << exception_to_string(__exc) << ": " << __msg << "\033[0m\n";
        }
        else
        {
            std::cerr << "\033[1;31m" << exception_to_string(__exc) << "\033[0m\n";
        }
        exit(__exc);
    }

    void raise(ExceptionStr __exc)
    {
        this->raise(__exc.exception, __exc.msg);
    }
};

Traceback __tb;

class Structure;
class Environment;

std::vector<Structure *> structers;

struct STR_DATA
{
    Structure *type = NULL;
    std::vector<STR_DATA> inner;
};

struct registry
{
    std::vector<uint8_t> nreg;
} reg;

using check_func = STR_DATA(size_t, const char *);
using detect_func = std::function<bool(STR_DATA *)>;
using react_func = std::function<void(Environment *, STR_DATA *)>;
using signal_func = std::function<void()>;

std::vector<STR_DATA> src;

class Structure
{
    react_func __reacter;

public:
    Structure(const char *__name, react_func __reacter)
    {
        this->__reacter = __reacter;
        structers.push_back(this);
    }

    void run(Environment *env, STR_DATA *str){
        this->__reacter(env, str);
    }
};

class Environment
{
    std::vector<signal_func> sig_handlers;
    std::vector<STR_DATA> main_struct;
    public:
    registry reg;
    std::vector<uint8_t> memory = {0};
    std::vector<size_t> pointers = {0};
    std::vector<Environment> functions;
    size_t selected_pointer = 0;
    Environment(std::vector<STR_DATA> main_struct){
        this->main_struct = main_struct;
    }
    void add_signal(signal_func handler)
    {
        sig_handlers.push_back(handler);
    }
    void signal(uint8_t sig)
    {
        if(sig_handlers.size() > sig) sig_handlers.at(sig)();
    }
    void run(){
        for(size_t i = 0; i < main_struct.size(); i++){
            main_struct[i].type->run(this, &main_struct[i]);
        }
    }
};

Structure ADD("ADD", [](Environment *env, STR_DATA *str){
        env->memory[env->pointers[env->selected_pointer]]++;
    });

Structure SUB("SUB", [](Environment *env, STR_DATA *str){
        env->memory[env->pointers[env->selected_pointer]]--;
    });

Structure LFT("LFT", [](Environment *env, STR_DATA *str){
        if(env->pointers[env->selected_pointer] <= 0){
            __tb.raise(MemoryUnderflow, "out of range");
            return;
        }
        env->pointers[env->selected_pointer]--;
    });

Structure RGT("RGT", [](Environment *env, STR_DATA *str){
        env->pointers[env->selected_pointer]++;
        while(env->pointers[env->selected_pointer] >= env->memory.size()){
            env->memory.push_back(0);
        }
    });

Structure INP("INP", [](Environment *env, STR_DATA *str){
        env->memory[env->pointers[env->selected_pointer]] = readKey();
    });

Structure OUT("OUT", [](Environment *env, STR_DATA *str){
        std::cout << env->memory[env->pointers[env->selected_pointer]];
    });

Structure N_POINTER("N_POINTER", [](Environment *env, STR_DATA *str){
        env->selected_pointer++;
        while(env->selected_pointer >= env->pointers.size()){
            env->pointers.push_back(env->pointers[env->selected_pointer-1]);
        }
    });

Structure P_POINTER("P_POINTER", [](Environment *env, STR_DATA *str){
        if(env->selected_pointer <= 0){
            __tb.raise(PointerUnderflow, "out of range");
            return;
        }
        env->selected_pointer--;
    });

Structure LOOP("LOOP", [](Environment *env, STR_DATA *str){
        STR_DATA aloc;
        while(env->memory[env->pointers[env->selected_pointer]]){
            for(size_t i = 0; i < str->inner.size(); i++){
                aloc = str->inner[i];
                str->inner[i].type->run(env, &aloc);
            }
        }
    });

Structure POINTER_LOOP("POINTER_LOOP", [](Environment *env, STR_DATA *str){
        STR_DATA aloc;
        while(env->pointers[env->selected_pointer]){
            for(size_t i = 0; i < str->inner.size(); i++){
                aloc = str->inner[i];
                str->inner[i].type->run(env, &aloc);
            }
        }
    });

Structure RET("RET", [](Environment *env, STR_DATA *str){
        env->signal(0);
    });

Structure PART("PART", [](Environment *env, STR_DATA *str){
        Environment venv(str->inner);
        venv.memory = env->memory;
        venv.pointers = env->pointers;
        venv.selected_pointer = env->selected_pointer;
        venv.functions = env->functions;
        venv.add_signal([&](){
            while(venv.pointers[venv.selected_pointer] >= env->memory.size())
                env->memory.push_back(0);
            env->memory[venv.pointers[venv.selected_pointer]] = venv.memory[venv.pointers[venv.selected_pointer]];
        });
        venv.run();
    });

Structure FUNC("FUNC", [](Environment *env, STR_DATA *str){
        Environment func(str->inner);
        func.functions = env->functions;
        env->functions.push_back(func);
        env->memory[env->pointers[env->selected_pointer]] = env->functions.size() - 1;
    });

Structure CALL_FUNC("CALL_FUNC", [](Environment *env, STR_DATA *str){
        if(env->memory[env->pointers[env->selected_pointer]] >= env->functions.size()){
            __tb.raise(Undefined, "function Undefined");
        }
        Environment inner(str->inner);
        inner.memory = env->memory;
        inner.pointers = env->pointers;
        inner.selected_pointer = env->selected_pointer;
        uint8_t cargs(0);
        inner.add_signal([&](){
            env->functions[env->memory[env->pointers[env->selected_pointer]]].memory.push_back(inner.memory[inner.pointers[inner.selected_pointer]]);
            cargs++;
        });
        inner.run();
        size_t creturn(0);
        env->functions[env->memory[env->pointers[env->selected_pointer]]].add_signal([&](){
            ++creturn;
            while(env->pointers[env->selected_pointer]+creturn >= env->memory.size())
            {
                env->memory.push_back(0);
            }
            env->memory.at(env->pointers[env->selected_pointer]+creturn) = env->functions[env->memory[env->pointers[env->selected_pointer]]].memory[env->functions[env->memory[env->pointers[env->selected_pointer]]].pointers[env->functions[env->memory[env->pointers[env->selected_pointer]]].selected_pointer]];
        });
        env->functions[env->memory[env->pointers[env->selected_pointer]]].memory.at(0) = cargs;
        env->functions[env->memory[env->pointers[env->selected_pointer]]].run();
        env->functions[env->memory[env->pointers[env->selected_pointer]]].memory.clear();
        env->functions[env->memory[env->pointers[env->selected_pointer]]].pointers.clear();
    });