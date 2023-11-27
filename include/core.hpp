#include <functional>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <iostream>
#include <vector>

#ifndef BFX_CORE_HEADER
#define BFX_CORE_HEADER

void g_writeKey(char);
char g_readKey();

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <filesystem>
#include <windows.h>
#include <conio.h>

HANDLE hConsole;
#endif

enum Exceptions
{
    Exception = 1,
    SyntaxError,
    MemoryOverflow,
    MemoryUnderflow,
    PointerOverflow,
    PointerUnderflow,
    Undefined
};

template <typename... REST>
char *string_format(const char *__msg, const REST &...args);

constexpr const char *exception_to_string(Exceptions e) throw()
{
    switch (e)
    {
    case Exceptions::SyntaxError:
        return "SyntaxError";
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
    bool throw_exc = true;
    Traceback(FILE *);
    void raise(Exceptions, const char *);
    void raise(ExceptionStr);
} __tb(stderr);

class Structure;
class Environment;

std::vector<Structure *> structers;

struct STR_DATA
{
    Structure *type = NULL;
    std::vector<STR_DATA> inner;
};

using check_func = std::function<STR_DATA(size_t, const char *)>;
using detect_func = std::function<bool(STR_DATA *)>;
using react_func = std::function<void(Environment *, STR_DATA *)>;
using signal_func = std::function<void()>;

std::vector<STR_DATA> src;

class Environment
{
    std::vector<signal_func> sig_handlers;
    std::vector<STR_DATA> structs;
public:
    std::vector<uint8_t> memory = {0};
    std::vector<size_t> pointers = {0};
    std::vector<Environment> functions;
    size_t selected_pointer = 0;
    Environment(std::vector<STR_DATA>);
    void add_signal(signal_func);
    void signal(uint8_t);
    void run();
};

class Structure
{
    react_func __reacter;

public:
    std::vector<STR_DATA> tree;
    Structure(react_func);
    void run(Environment *, STR_DATA *);
};
#endif
