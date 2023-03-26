#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <functional>
#include <string>
#include <fstream>
#include "path.hpp"

#ifndef BF
#define BF
#define INITIAL_REGISTRY_ARGS 2

void g_writeKey(char _val)
{
    std::cout << _val;
}
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <filesystem>
#include <conio.h>
char g_readKey()
{
    return (char)_getch();
}
void clear()
{
    system("cls");
}
#else
char g_readKey()
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

struct line_addr
{
    size_t line = 0;
    size_t offset = 0;
};

class LineAddr
{
    char *__msg;

public:
    LineAddr(char *msg) : __msg(msg) {}
    line_addr get_line(size_t __offset)
    {
        line_addr adder;
        if (__offset > strlen(__msg))
        {
            throw std::runtime_error("offset so large");
        }
        adder.line = 1;
        for (size_t __index = 0; __index < __offset; __index++)
        {
            if (__msg[__index] == '\n')
            {
                adder.line++;
                adder.offset = 0;
            }
            else
            {
                adder.offset++;
            }
        }
        return adder;
    }
};

LineAddr *__line_adder;

template <typename... REST>
char *string_format(const char *__msg, const REST &...args)
{
    static char buffer[1024];
    snprintf(buffer, sizeof(buffer), __msg, args...);
    return buffer;
}

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
    size_t start, end;
    Structure *type = NULL;
    bool ignore = false;
    std::vector<STR_DATA> inner;
};

struct registry
{
    std::vector<uint8_t> nreg;
} reg;

using check_func = STR_DATA(size_t, const char *);
using detect_func = std::function<bool(STR_DATA *)>;
using react_func = std::function<void(Environment *, STR_DATA *)>;
using build_func = std::function<std::string(STR_DATA *)>;
using signal_func = std::function<void()>;

std::vector<STR_DATA> src;

class Structure
{
    check_func *__checker;
    react_func __reacter;
    build_func __builder;

public:
    std::vector<STR_DATA> tree;
    const char *name = "MAIN";
    static std::vector<STR_DATA> detector(size_t __index, size_t __eof, const char *__src, detect_func __handler = NULL)
    {
        std::vector<STR_DATA> tree;
        bool is_handled = false;
        STR_DATA res;
        bool closed = false;
        while (__index < __eof)
        {
            for (uint8_t i = 0; i < structers.size(); i++)
            {
                res = structers[i]->__checker(__index, __src);
                if (res.type != NULL)
                {
                    if (__handler != NULL)
                    {
                        closed = __handler(&res);
                    }
                    if (!res.ignore)
                    {
                        tree.push_back(res);
                    }
                    __index = res.end + 1;
                    is_handled = true;
                    break;
                }
            }
            if (is_handled)
            {
                is_handled = false;
            }
            else
            {
                __index++;
            }
            if (closed)
            {
                break;
            }
        }
        return tree;
    }

    Structure(const char *__src);
    Structure(const char *__name, check_func *__checker, react_func __reacter, build_func __builder) : name(__name)
    {
        this->__checker = __checker;
        this->__reacter = __reacter;
        this->__builder = __builder;
        structers.push_back(this);
    }

    void run(Environment *env, STR_DATA *str){
        this->__reacter(env, str);
    }

    std::string build(STR_DATA *str)
    {
        return this->__builder(str);
    }
};

class iostream
{
    std::string _out, _inp;
    bool check_is_allow_to_read()
    {
        if (_inp.length() > 0)
            return true;
        return false;
    }
    public:
    uint operator<<(std::string _val)
    {
        _inp += _val;
        return _val.length();
    }
    uint operator<<(char *_val)
    {
        _inp += _val;
        return strlen(_val);
    }
    uint operator<<(char _val)
    {
        _inp += _val;
        return 1;
    }
    uint operator>>(std::string &_val)
    {
        _val = _out;
        _out = "";
        return _val.length();
    }
    uint operator>>(char &_val)
    {
        if (_out.length() > 0){
            _val = *(char *)_out.substr(0, 1).c_str();
            _out.erase(0, 1);
            return 1;
        }
        return 0;
    }
    void read(char &_val)
    {
        this->operator>>(_val);
    }
    void read(char *&_val, uint _size)
    {
        _val = (char *)_out.substr(0, _size).c_str();
        _out.erase(0, _size);
    }
    void read(std::string &_val, uint _size)
    {
        _val = _out.substr(0, _size);
        _out.erase(0, _size);
    }
    uint write(char _val)
    {
        _inp += _val;
        return 1;
    }
    uint write(char *_val, uint _size)
    {
        for (size_t _ind = 0; _ind < _size; _ind++)
        {
            _inp += _val[_ind];
        }
        return _size;
    }
    uint write(std::string _val, uint _size)
    {
        for (size_t _ind = 0; _ind < _size; _ind++)
        {
            _inp += _val[_ind];
        }
        return _size;
    }
    friend std::string read_iostream(iostream *);
    friend std::string read_iostream(iostream *, uint);
    friend uint write_iostream(iostream *, char);
    friend uint write_iostream(iostream *, char *, uint);
    friend uint write_iostream(iostream *, std::string);
};

std::string read_iostream(iostream *_stream)
{
    while (_stream->check_is_allow_to_read()){}
    std::string res = _stream->_inp;
    _stream->_inp = "";
    return res;
}
std::string read_iostream(iostream *_stream, uint _size)
{
    while (!_stream->check_is_allow_to_read()){}
    std::string res = _stream->_inp.substr(0, _size);
    _stream->_inp.erase(0, _size);
    return res;
}
uint write_iostream(iostream *_stream, char _val)
{
    _stream->_out += _val;
    return 1;
}
uint write_iostream(iostream *_stream, char *_val, uint _size)
{
    for (size_t _ind = 0; _ind < _size; _ind++)
    {
        _stream->_out += _val[_ind];
    }
    return _size;
}
uint write_iostream(iostream *_stream, std::string _val)
{
    _stream->_out += _val;
    return _val.length();
}

class Environment
{
    std::vector<signal_func> sig_handlers;
    STR_DATA main_struct;
    public:
    registry reg;
    std::vector<uint8_t> memory = {0};
    std::vector<size_t> pointers = {0};
    std::vector<Environment> functions;
    size_t selected_pointer = 0;
    std::function<char()> readKey = g_readKey;
    std::function<void(char)> writeKey = g_writeKey;
    iostream *_stream = NULL;
    std::function<void()> _on_read, _on_write;
    Environment(STR_DATA main_struct, iostream *_stream = NULL, std::function<void()> _on_read = [](){}, std::function<void()> _on_write = [](){}){
        this->main_struct = main_struct;
        this->_on_read = _on_read;
        this->_on_write = _on_write;
        if (_stream != NULL)
        {
            this->_stream = _stream;
            readKey = [&]() -> char{
                if (this->_on_read != NULL)
                    this->_on_read();
                return *(char *)read_iostream(this->_stream, 1).c_str();
            };
            writeKey = [&](char _val) -> void{
                if (_on_read != NULL)
                    this->_on_write();
                write_iostream(this->_stream, _val);
            };
        }
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
        main_struct.type->run(this, &main_struct);
    }
    std::string build(){
        return main_struct.type->build(&main_struct);
    }
};

char *__source;

Structure S_ADD("ADD", [](size_t __index, const char *__src) -> STR_DATA
              {
    if(__src[__index] == '+'){
        return {__index, __index, &S_ADD};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        env->memory[env->pointers[env->selected_pointer]]++;
    }, [](STR_DATA *str) -> std::string{
        return "{&S_ADD}";
    });

Structure S_SUB("SUB", [](size_t __index, const char *__src) -> STR_DATA
              {
    if(__src[__index] == '-'){
        return {__index, __index, &S_SUB};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        env->memory[env->pointers[env->selected_pointer]]--;
    }, [](STR_DATA *str) -> std::string{
        return "{&S_SUB}";
    });

Structure S_LFT("LFT", [](size_t __index, const char *__src) -> STR_DATA
              {
    if(__src[__index] == '<'){
        return {__index, __index, &S_LFT};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        if(env->pointers[env->selected_pointer] <= 0){
            line_addr adder = __line_adder->get_line(str->start + 1);
            __tb.raise(MemoryUnderflow, string_format("out of range at %i:%i", adder.line, adder.offset));
            return;
        }
        env->pointers[env->selected_pointer]--;
    }, [](STR_DATA *str) -> std::string{
        return "{&S_LFT}";
    });

Structure S_RGT("RGT", [](size_t __index, const char *__src) -> STR_DATA
              {
    if(__src[__index] == '>'){
        return {__index, __index, &S_RGT};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        env->pointers[env->selected_pointer]++;
        while(env->pointers[env->selected_pointer] >= env->memory.size()){
            env->memory.push_back(0);
        }
    }, [](STR_DATA *str) -> std::string{
        return "{&S_RGT}";
    });

Structure S_INP("INP", [](size_t __index, const char *__src) -> STR_DATA
              {
    if(__src[__index] == ','){
        return {__index, __index, &S_INP};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        env->memory[env->pointers[env->selected_pointer]] = env->readKey();
    }, [](STR_DATA *str) -> std::string{
        return "{&S_INP}";
    });

Structure S_OUT("OUT", [](size_t __index, const char *__src) -> STR_DATA
              {
    if(__src[__index] == '.'){
        return {__index, __index, &S_OUT};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        env->writeKey(env->memory[env->pointers[env->selected_pointer]]);
    }, [](STR_DATA *str) -> std::string{
        return "{&S_OUT}";
    });

Structure S_N_POINTER("N_POINTER", [](size_t __index, const char *__src) -> STR_DATA
                    {
    if(__src[__index] == '/'){
        return {__index, __index, &S_N_POINTER};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        env->selected_pointer++;
        while(env->selected_pointer >= env->pointers.size()){
            env->pointers.push_back(env->pointers[env->selected_pointer-1]);
        }
    }, [](STR_DATA *str) -> std::string{
        return "{&S_N_POINTER}";
    });

Structure S_P_POINTER("P_POINTER", [](size_t __index, const char *__src) -> STR_DATA
                    {
    if(__src[__index] == '\\'){
        return {__index, __index, &S_P_POINTER};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        if(env->selected_pointer <= 0){
            line_addr adder = __line_adder->get_line(str->start + 1);
            __tb.raise(PointerUnderflow, string_format("out of range at %i:%i", adder.line, adder.offset));
            return;
        }
        env->selected_pointer--;
    }, [](STR_DATA *str) -> std::string{
        return "{&S_P_POINTER}";
    });

Structure END_LOOP("END_LOOP", [](size_t __index, const char *__src) -> STR_DATA
                   {
    if(__src[__index] == ']'){
        return {__index, __index, &END_LOOP, true};
    }else{
        return {};
    } }, NULL, NULL);

Structure END_POINTER_LOOP("END_POINTER_LOOP", [](size_t __index, const char *__src) -> STR_DATA
                           {
    if(__src[__index] == '}'){
        return {__index, __index, &END_POINTER_LOOP, true};
    }else{
        return {};
    } }, NULL, NULL);

Structure END_PART("END_PART", [](size_t __index, const char *__src) -> STR_DATA
                   {
    if(__src[__index] == ')'){
        return {__index, __index, &END_PART, true};
    }else{
        return {};
    } }, NULL, NULL);

Structure S_LOOP("LOOP", [](size_t __index, const char *__src) -> STR_DATA
               {
    if(__src[__index] == '['){
        size_t _start = __index, _end;
        bool is_ended = false;
        line_addr adder;

        STR_DATA res = {_start};
        res.type = &S_LOOP;
        res.inner = Structure::detector(__index+1, strlen(__src), __src, [&](STR_DATA *__str) -> bool{
            if(__str->type == &END_POINTER_LOOP){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("'}' without '{' at %i:%i", adder.line, adder.offset));
            }else if(__str->type == &END_PART){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("')' without '(' at %i:%i", adder.line, adder.offset));
            }else if(__str->type == &END_LOOP){
                _end = __str->end;
                is_ended = true;
                return true;
            }
            return false;
        });
        if(!is_ended){
            adder = __line_adder->get_line(__index+1);
            __tb.raise(SyntaxError, string_format("Missing ']' at %i:%i", adder.line, adder.offset));
            return {};
        }
        res.end = _end;
        return res;

    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        STR_DATA aloc;
        while(env->memory[env->pointers[env->selected_pointer]]){
            for(size_t i = 0; i < str->inner.size(); i++){
                aloc = str->inner[i];
                str->inner[i].type->run(env, &aloc);
            }
        }
    }, [](STR_DATA *str) -> std::string{
        std::string res = "{&S_LOOP,{";
        for(size_t i = 0; i < str->inner.size(); i++){
            res += str->inner[i].type->build(&str->inner[i]) + ",";
        }
        res += "}}";
        return res;
    });

Structure S_POINTER_LOOP("POINTER_LOOP", [](size_t __index, const char *__src) -> STR_DATA
                       {
    if(__src[__index] == '{'){
        size_t _start = __index, _end;
        bool is_ended = false;
        line_addr adder;

        STR_DATA res = {_start};
        res.type = &S_POINTER_LOOP;
        res.inner = Structure::detector(__index+1, strlen(__src), __src, [&](STR_DATA *__str) -> bool{
            if(__str->type == &END_LOOP){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("']' without '[' at %i:%i", adder.line, adder.offset));
            }else if(__str->type == &END_PART){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("')' without '(' at %i:%i", adder.line, adder.offset));
            }else if(__str->type == &END_POINTER_LOOP){
                _end = __str->end;
                is_ended = true;
                return true;
            }
            return false;
        });
        if(!is_ended){
            adder = __line_adder->get_line(__index+1);
            __tb.raise(SyntaxError, string_format("Missing '}' at %i:%i", adder.line, adder.offset));
            return {};
        }
        res.end = _end;
        return res;
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        STR_DATA aloc;
        while(env->pointers[env->selected_pointer]){
            for(size_t i = 0; i < str->inner.size(); i++){
                aloc = str->inner[i];
                str->inner[i].type->run(env, &aloc);
            }
        }
    }, [](STR_DATA *str) -> std::string{
        std::string res = "{&S_POINTER_LOOP,{";
        for(size_t i = 0; i < str->inner.size(); i++){
            res += str->inner[i].type->build(&str->inner[i]) + ",";
        }
        res += "}}";
        return res;
    });

Structure S_RET("RET", [](size_t __index, const char *__src) -> STR_DATA
                   {
    if(__src[__index] == '&'){
        return {__index, __index, &S_RET};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        env->signal(0);
    }, [](STR_DATA *str) -> std::string{
        return "{&S_RET}";
    });

Structure S_PART("PART", [](size_t __index, const char *__src) -> STR_DATA
                   {
    if(__src[__index] == '('){
        size_t _start = __index, _end;
        bool is_ended = false;
        line_addr adder;

        STR_DATA res = {_start};
        res.type = &S_PART;
        res.inner = Structure::detector(__index+1, strlen(__src), __src, [&](STR_DATA *__str) -> bool{
            if(__str->type == &END_LOOP){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("']' without '[' at %i:%i", adder.line, adder.offset));
            }else if(__str->type == &END_POINTER_LOOP){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("'}' without '{' at %i:%i", adder.line, adder.offset));
            }else if(__str->type == &END_PART){
                _end = __str->end;
                is_ended = true;
                return true;
            }
            return false;
        });
        if(!is_ended){
            adder = __line_adder->get_line(__index+1);
            __tb.raise(SyntaxError, string_format("Missing ')' at %i:%i", adder.line, adder.offset));
            return {};
        }
        res.end = _end;
        return res;
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        Environment venv(*str);
        venv.memory = env->memory;
        venv.pointers = env->pointers;
        venv.selected_pointer = env->selected_pointer;
        venv.functions = env->functions;
        venv.add_signal([&](){
            while(venv.pointers[venv.selected_pointer] >= env->memory.size())
                env->memory.push_back(0);
            env->memory[venv.pointers[venv.selected_pointer]] = venv.memory[venv.pointers[venv.selected_pointer]];
        });
        venv.readKey = env->readKey;
        venv.writeKey = env->writeKey;
        venv.run();
    }, [](STR_DATA *str) -> std::string{
        std::string res = "{&S_PART,{";
        for(size_t i = 0; i < str->inner.size(); i++){
            res += str->inner[i].type->build(&str->inner[i]) + ",";
        }
        res += "}}";
        return res;
    });

Structure END_FUNC("END_FUNC", [](size_t __index, const char *__src) -> STR_DATA
                   {
    return {};
    }, NULL, NULL);

Structure S_FUNC("FUNC", [](size_t __index, const char *__src) -> STR_DATA
                   {
    if(__src[__index] == '%'){
        if(reg.nreg.at(0))
        {
            return {__index, __index, &END_FUNC};
        }
        reg.nreg.at(0) = 1;
        size_t _start = __index, _end;
        bool is_ended = false;
        line_addr adder;

        STR_DATA res = {_start};
        res.type = &S_FUNC;
        res.inner = Structure::detector(__index+1, strlen(__src), __src, [&](STR_DATA *__str) -> bool{
            if(__str->type == &END_LOOP){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("']' without '[' at %i:%i", adder.line, adder.offset));
            }else if(__str->type == &END_POINTER_LOOP){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("'}' without '{' at %i:%i", adder.line, adder.offset));
            }else if(__str->type == &END_PART){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("')' without '(' at %i:%i", adder.line, adder.offset));
            }else if(__str->type == &END_FUNC){
                _end = __str->end;
                is_ended = true;
                return true;
            }
            return false;
        });
        if(!is_ended){
            adder = __line_adder->get_line(__index+1);
            __tb.raise(SyntaxError, string_format("Function not closed at %i:%i", adder.line, adder.offset));
            return {};
        }
        res.end = _end;
        reg.nreg.at(0) = 0;
        return res;
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        Environment func(*str);
        func.readKey = env->readKey;
        func.writeKey = env->writeKey;
        func.functions = env->functions;
        env->functions.push_back(func);
        env->memory[env->pointers[env->selected_pointer]] = env->functions.size() - 1;
    }, [](STR_DATA *str) -> std::string{
        std::string res = "{&S_FUNC,{";
        for(size_t i = 0; i < str->inner.size(); i++){
            res += str->inner[i].type->build(&str->inner[i]) + ",";
        }
        res += "}}";
        return res;
    });

Structure END_CALL_FUNC("END_CALL_FUNC", [](size_t __index, const char *__src) -> STR_DATA
                   {
    return {};
    }, NULL, NULL);

Structure S_CALL_FUNC("CALL_FUNC", [](size_t __index, const char *__src) -> STR_DATA
                   {
    if(__src[__index] == '$'){
        if(reg.nreg.at(1))
        {
            return {__index, __index, &END_CALL_FUNC};
        }
        reg.nreg.at(1) = 1;
        size_t _start = __index, _end;
        bool is_ended = false;
        line_addr adder;

        STR_DATA res = {_start};
        res.type = &S_CALL_FUNC;
        res.inner = Structure::detector(__index+1, strlen(__src), __src, [&](STR_DATA *__str) -> bool{
            if(__str->type == &END_LOOP){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("']' without '[' at %i:%i", adder.line, adder.offset));
            }else if(__str->type == &END_POINTER_LOOP){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("'}' without '{' at %i:%i", adder.line, adder.offset));
            }else if(__str->type == &END_PART){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("')' without '(' at %i:%i", adder.line, adder.offset));
            }else if(__str->type == &S_FUNC){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("illegal function definition at %i:%i", adder.line, adder.offset));
            }else if(__str->type == &END_CALL_FUNC){
                _end = __str->end;
                is_ended = true;
                return true;
            }
            return false;
        });
        if(!is_ended){
            adder = __line_adder->get_line(__index+1);
            __tb.raise(SyntaxError, string_format("function not closed at %i:%i", adder.line, adder.offset));
            return {};
        }
        res.end = _end;
        reg.nreg.at(1) = 0;
        return res;
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        if(env->memory[env->pointers[env->selected_pointer]] >= env->functions.size()){
            __line_adder = new LineAddr(__source);
            line_addr adder = __line_adder->get_line(str->start);
            __tb.raise(Undefined, string_format("function Undefined at %i:%i", adder.line, adder.offset));
        }
        Environment inner(*str);
        inner.memory = env->memory;
        inner.pointers = env->pointers;
        inner.selected_pointer = env->selected_pointer;
        uint8_t cargs(0);
        inner.add_signal([&](){
            env->functions[env->memory[env->pointers[env->selected_pointer]]].memory.push_back(inner.memory[inner.pointers[inner.selected_pointer]]);
            cargs++;
        });
        inner.readKey = env->readKey;
        inner.writeKey = env->writeKey;
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
    }, [](STR_DATA *str) -> std::string{
        std::string res = "{&S_CALL_FUNC,{";
        for(size_t i = 0; i < str->inner.size(); i++){
            res += str->inner[i].type->build(&str->inner[i]) + ",";
        }
        res += "}}";
        return res;
    });

Structure::Structure(const char *__src)
{
    __source = (char *)__src;
    __line_adder = new LineAddr((char *)__src);
    line_addr adder;
    this->tree = this->detector(0, strlen(__src), __src, [&](STR_DATA *__str) -> bool
                                {
            if(__str->type == &END_LOOP){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("']' without '[' at %i:%i", adder.line, adder.offset));
            }else if(__str->type == &END_POINTER_LOOP){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("'}' without '{' at %i:%i", adder.line, adder.offset));
            }else if(__str->type == &END_PART){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("')' without '(' at %i:%i", adder.line, adder.offset));
            }
            return false; });
    this->__builder = [&](STR_DATA *str) -> std::string{
        std::string src = "#include \"core.hpp\"\nstatic std::vector<STR_DATA> TREE = {";
        for(size_t _ind = 0; _ind < str->inner.size(); _ind++)
        {
            src += str->inner[_ind].type->build(&str->inner[_ind]) + ',';
        }
        src += "};\n#include \"start.hpp\"";
        return src;
    };
    this->__reacter = [](Environment *env, STR_DATA *str){
        for(size_t _ind = 0; _ind < str->inner.size(); _ind++)
        {
            str->inner[_ind].type->run(env, &str->inner[_ind]);
        }
    };
}
#endif