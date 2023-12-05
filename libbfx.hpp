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

void g_writeKey(char);
char g_readKey();

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <filesystem>
#include <windows.h>
#include <conio.h>
#else
#include <dlfcn.h>
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

constexpr const char *exception_to_string(Exceptions e) throw();

struct ExceptionStr
{
    Exceptions exception;
    char *msg;
};

void bfx_exit(int);

class Traceback
{
    FILE *__stderr;
public:
    Traceback(FILE *);
    void raise(Exceptions, const char *);
    void raise(ExceptionStr);
};

class Structure;
class Environment;
class iostream;

struct STR_DATA
{
    size_t start, end;
    Structure *type = NULL;
    bool ignore = false;
    std::vector<STR_DATA> inner;
};

class registry_value
{
    private:
    
    uint64_t *reg_ptr;

    public:

    registry_value(uint64_t *);
    uint64_t operator=(uint64_t);
    uint64_t operator++();
    uint64_t operator--();
    uint64_t operator++(int);
    uint64_t operator--(int);
    uint64_t operator+=(uint64_t);
    uint64_t operator-=(uint64_t);
    operator bool();
    bool operator!();
};

class registry
{
    protected:

    uint64_t *mem, mem_size;
    void resize(uint64_t);

    public:

    registry();
    registry_value operator[](uint64_t);
    uint64_t get(uint64_t);
    void set(uint64_t, uint64_t);
    ~registry();
};

using check_func = std::function<STR_DATA(size_t, const char *)>;
using detect_func = std::function<bool(STR_DATA *)>;
using react_func = std::function<void(Environment *, STR_DATA *)>;
using build_func = std::function<std::string(STR_DATA *)>;
using signal_func = std::function<void()>;

typedef void (*load_structures_func)(std::vector<Structure *> *);

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
    Structure(const char *, check_func, build_func, react_func);

    void run(Environment *, STR_DATA *);

    std::string build(STR_DATA *);
};

Structure *get_structure(const char *);

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

class Program : public Environment
{
    Structure *main_struct;
public:
    Program(Structure *, iostream *, std::function<void()>, std::function<void()>);
};

#ifdef BFX_PLUGIN_BUILD

extern "C"
{
    void load_structures(std::vector<Structure *> *);
}

#else

void load_plugin(const char *, bool);

#endif
#endif
