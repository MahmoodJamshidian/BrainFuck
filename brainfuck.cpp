#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <functional>
#include <string>
#include <fstream>
#include "path.hpp"

#define INITIAL_REGISTRY_ARGS 2

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <filesystem>
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
    Environment(STR_DATA main_struct){
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
        main_struct.type->run(this, &main_struct);
    }
    std::string build(){
        return main_struct.type->build(&main_struct);
    }
};

char *__src;

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
        env->memory[env->pointers[env->selected_pointer]] = readKey();
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
        std::cout << env->memory[env->pointers[env->selected_pointer]];
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
            __line_adder = new LineAddr(__src);
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

void show_version()
{
    std::cout << "BrainFuck version 0.4\nCopyright (c) 2023 Mahmood Jamshidian (MJScript)\nMIT License\n\nWritten by Mahmood Jamshidian (MJScript)\n\nread more at https://github.com/MahmoodJamshidian/BrainFuck\n";
}

void show_help(char const *argv[])
{
    std::string app_name = argv[0];
    app_name = app_name.substr(app_name.find_last_of("/") + 1);
    std::cout << "Usage: " << app_name << " <source> [options]\nOptions:\n  -h --help\t\tShow help\n  -v --version\t\tShow version\n  (The following options are related to the build)\n  -b --build\t\tBuild executable file\n  -o <output file>\tThe output path of the executable file (source path by default)\n  -c <c++ compiler>\tSet c++ compiler (g++ by default)\n  --option <option>\tSet c++ compiler options" << std::endl;
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        show_help(argv);
        return Exception;
    }
    if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)
    {
        show_version();
        return 0;
    }
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
        show_help(argv);
        return 0;
    }
    char const *filename = argv[1];
    const char *ext = std::string(filename).substr(std::string(filename).find_last_of(".") + 1).c_str();
    if (strcmp(ext, "bf") != 0 && strcmp(ext, "b") != 0)
    {
        __tb.raise(Exception, "invalid file extension");
        return Exception;
    }
    bool is_build = false;
    std::string output = "";
    std::string compiler = "";
    std::string options = "";

    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "-b") == 0)
        {
            if (is_build)
            {
                __tb.raise(Exception, string_format("repeat '%s' switch", argv[i]));
                return Exception;
            }
            is_build = true;
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            if (output != "")
            {
                __tb.raise(Exception, string_format("repeated '%s' switch", argv[i]));
                return Exception;
            }
            if (++i<argc)
            {
                output = argv[i];
            }
            else
            {
                __tb.raise(Exception, string_format("'%s' switch was ignored", argv[i-1]));
                return Exception;
            }
        }
        else if (strcmp(argv[i], "-c") == 0)
        {
            if (compiler != "")
            {
                __tb.raise(Exception, string_format("repeated '%s' switch", argv[i]));
                return Exception;
            }
            if (++i<argc)
            {
                compiler = argv[i];
            }
            else
            {
                __tb.raise(Exception, string_format("'%s' switch was ignored", argv[i-1]));
                return Exception;
            }
        }
        else if (strcmp(argv[i], "--option") == 0)
        {
            if (++i<argc)
            {
                options += " ";
                options += argv[i];
            }
            else
            {
                __tb.raise(Exception, string_format("'%s' switch was ignored", argv[i-1]));
                return Exception;
            }
        }
        else
        {
            __tb.raise(Exception, string_format("invalid command '%s'", argv[i]));
            return Exception;
        }
    }
    if (options == "")
        options = " ";
    else if (!is_build)
    {
        __tb.raise(Exception, "switch '--option' in run mode");
        return Exception;
    }
    if (!is_build && output != "")
    {
        __tb.raise(Exception, "switch '-o' in run mode");
        return Exception;
    }
    if (is_build && output == "")
    {
        output = std::string(filename).substr(0, std::string(filename).find_last_of("."));
    }
    if (compiler == "")
        compiler = "g++";
    std::ifstream file;
    file.open(filename, std::ifstream::in | std::ifstream::ate);

    if(file.fail()){
        __tb.raise(IOError, string_format("can't open '%s': %s", filename, strerror(errno)));
        return IOError;
    }
    char *code = (char *)malloc(file.tellg() * sizeof(char));
    file.seekg(0);
    char c;
    size_t index = 0;
    while (true)
    {
        file.read(&c, 1);
        if(!file.eof()){
            if(c != '\r')
            {
                code[index] = c;
            }
            index++;
        }else{
            break;
        }
    }
    file.close();
    for(int _ = 0; _ < INITIAL_REGISTRY_ARGS; _++)
        reg.nreg.push_back(0);
    __src = code;
    Structure main_struct((const char *)code);
    Environment program({0, strlen(__src), &main_struct, false, main_struct.tree});
    if (!is_build)
        program.run();
    else
    {
        std::string __bsrc = program.build();
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        std::string bfile_addr = std::filesystem::temp_directory_path().string() + "bf-temp.cpp";
        std::ofstream bfile(bfile_addr.c_str());
        bool is_bfile_writable = (bool)bfile;
        bfile.write(__bsrc.c_str(), __bsrc.length());
        bfile.close();
        if (!is_bfile_writable)
        {
            __tb.raise(IOError, string_format("can't wtite build file in %s", bfile_addr));
            return IOError;
        }
        std::string cmd = compiler + " -Wall -I" + path::getExecutableDir() + "include -o " + output + options + ' ' + bfile_addr;
#else
        std::string cmd = compiler + " -Wall -I" + path::getExecutableDir() + "include -o " + output + options + " -xc++ - << EOF\n" + __bsrc + "\nEOF";
#endif
        bool is_worked = !system(cmd.c_str());
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        remove(bfile_addr.c_str());
#endif
        if (!is_worked)
        {
            __tb.raise(CompileError, "a compilation error occurred");
            return CompileError;
        }
    }
    return 0;
}
