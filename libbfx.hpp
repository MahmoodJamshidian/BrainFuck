#include <functional>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "path.hpp"
#include <fstream>
#include <vector>
#include <string>

#ifndef BFX_HEADER
#define BFX_HEADER

uint64_t INITIAL_REGISTRY_ARGS = 5;

void g_writeKey(char);
char g_readKey();

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <filesystem>
#include <windows.h>
#include <conio.h>
#endif

struct line_addr
{
    size_t line = 0;
    size_t offset = 0;
};

class LineAddr
{
    char *__msg;
public:
    LineAddr(char *);
    line_addr get_line(size_t);
};

LineAddr *__line_adder;

template <typename... REST>
char *string_format(const char *, const REST &...);

enum Exceptions
{
    Exception = 1,
    SyntaxError,
    MemoryOverflow,
    MemoryUnderflow,
    PointerOverflow,
    PointerUnderflow,
    Undefined,
    CompileError,
    IOError,
    InternalError
};

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
    case Exceptions::CompileError:
        return "CompileError";
    case Exceptions::IOError:
        return "IOError";
    case Exceptions::InternalError:
        return "InternalError";
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
class iostream;

std::vector<Structure *> structers;

struct STR_DATA
{
    size_t start, end;
    Structure *type = NULL;
    bool ignore = false;
    std::vector<STR_DATA> inner;
};

struct registry
{
    std::vector<uint8_t> nreg;

    registry(size_t _size)
    {
    	nreg.resize(_size);
    }
} reg(INITIAL_REGISTRY_ARGS);

using check_func = std::function<STR_DATA(size_t, const char *)>;
using detect_func = std::function<bool(STR_DATA *)>;
using react_func = std::function<void(Environment *, STR_DATA *)>;
using build_func = std::function<std::string(STR_DATA *)>;
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
    std::function<char()> readKey = g_readKey;
    std::function<void(char)> writeKey = g_writeKey;
    iostream *_stream = NULL;
    std::function<void()> _on_read, _on_write;
    Environment(std::vector<STR_DATA>, iostream *, std::function<void()>, std::function<void()>);
    void add_signal(signal_func);
    void signal(uint8_t);
    void run();
    std::string build();
};

class Structure
{
    check_func __checker;
    react_func __reacter;
    build_func __builder;

public:
    std::vector<STR_DATA> tree;
    const char *name = "MAIN";
    static std::vector<STR_DATA> detector(size_t, size_t, const char *, detect_func);

    Structure(const char *);
    Structure(const char *, check_func, react_func, build_func);

    void run(Environment *, STR_DATA *);

    std::string build(STR_DATA *);
};

class iostream
{
    std::string _out, _inp;
    bool check_is_allow_to_read();
public:
    size_t operator<<(std::string);
    size_t operator<<(char *);
    size_t operator<<(char);
    size_t operator>>(std::string &);
    size_t operator>>(char &);
    void read(char &);
    void read(char *&, size_t);
    void read(std::string &, size_t);
    size_t write(char);
    size_t write(char *, size_t);
    size_t write(std::string, size_t);
    friend std::string read_iostream(iostream *);
    friend std::string read_iostream(iostream *, size_t);
    friend size_t write_iostream(iostream *, char);
    friend size_t write_iostream(iostream *, char *, size_t);
    friend size_t write_iostream(iostream *, std::string);
};

char *__source;

class Program : public Environment
{
    Structure *main_struct;
public:
    Program(Structure *, iostream *, std::function<void()>, std::function<void()>);
};
#endif
