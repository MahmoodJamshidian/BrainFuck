#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <functional>
#include <string>

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
class Program;

std::vector<Structure *> structers;

struct STR_DATA
{
    size_t start, end;
    Structure *type = NULL;
    bool ignore = false;
    std::vector<STR_DATA> inner;
};

using check_func = STR_DATA(size_t, const char *);
using detect_func = std::function<bool(STR_DATA *)>;
using react_func = std::function<void(Program *, STR_DATA *)>;

std::vector<STR_DATA> src;

class Structure
{
    check_func *__checker;
    react_func __reacter;

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
    Structure(const char *__name, check_func *__checker, react_func __reacter) : name(__name)
    {
        this->__checker = __checker;
        this->__reacter = __reacter;
        structers.push_back(this);
    }

    void run(Program *program, STR_DATA *str){
        this->__reacter(program, str);
    }
};

class Program
{
    public:
    std::vector<uint8_t> memory = {0};
    std::vector<size_t> pointers = {0};
    size_t selected_pointer = 0;
    Program(std::vector<STR_DATA> main_struct){
        for(size_t i = 0; i < main_struct.size(); i++){
            main_struct[i].type->run(this, &main_struct[i]);
        }
    }
};

Structure ADD("ADD", [](size_t __index, const char *__src) -> STR_DATA
              {
    if(__src[__index] == '+'){
        return {__index, __index, &ADD};
    }else{
        return {};
    } }, [](Program *program, STR_DATA *str){
        program->memory[program->pointers[program->selected_pointer]]++;
    });

Structure SUB("SUB", [](size_t __index, const char *__src) -> STR_DATA
              {
    if(__src[__index] == '-'){
        return {__index, __index, &SUB};
    }else{
        return {};
    } }, [](Program *program, STR_DATA *str){
        program->memory[program->pointers[program->selected_pointer]]--;
    });

Structure LFT("LFT", [](size_t __index, const char *__src) -> STR_DATA
              {
    if(__src[__index] == '<'){
        return {__index, __index, &LFT};
    }else{
        return {};
    } }, [](Program *program, STR_DATA *str){
        if(program->pointers[program->selected_pointer] <= 0){
            line_addr adder = __line_adder->get_line(str->start + 1);
            __tb.raise(MemoryUnderflow, string_format("out of range at %i:%i", adder.line, adder.offset));
            return;
        }
        program->pointers[program->selected_pointer]--;
    });

Structure RGT("RGT", [](size_t __index, const char *__src) -> STR_DATA
              {
    if(__src[__index] == '>'){
        return {__index, __index, &RGT};
    }else{
        return {};
    } }, [](Program *program, STR_DATA *str){
        if(program->pointers[program->selected_pointer] >= program->memory.max_size() - 1){
            line_addr adder = __line_adder->get_line(str->start + 1);
            __tb.raise(MemoryOverflow, string_format("out of range at %i:%i", adder.line, adder.offset));
            return;
        }
        program->pointers[program->selected_pointer]++;
        while(program->pointers[program->selected_pointer] >= program->memory.size()){
            program->memory.push_back(0);
        }
    });

Structure INP("INP", [](size_t __index, const char *__src) -> STR_DATA
              {
    if(__src[__index] == ','){
        return {__index, __index, &INP};
    }else{
        return {};
    } }, [](Program *program, STR_DATA *str){
        program->memory[program->pointers[program->selected_pointer]] = readKey();
    });

Structure OUT("OUT", [](size_t __index, const char *__src) -> STR_DATA
              {
    if(__src[__index] == '.'){
        return {__index, __index, &OUT};
    }else{
        return {};
    } }, [](Program *program, STR_DATA *str){
        std::cout << program->memory[program->pointers[program->selected_pointer]];
    });

Structure N_POINTER("N_POINTER", [](size_t __index, const char *__src) -> STR_DATA
                    {
    if(__src[__index] == '/'){
        return {__index, __index, &N_POINTER};
    }else{
        return {};
    } }, [](Program *program, STR_DATA *str){
        if(program->selected_pointer >= program->pointers.max_size() - 1){
            line_addr adder = __line_adder->get_line(str->start + 1);
            __tb.raise(PointerOverflow, string_format("out of range at %i:%i", adder.line, adder.offset));
            return;
        }
        while(program->selected_pointer >= program->pointers.size()){
            program->pointers.push_back(program->pointers[program->selected_pointer]);
        }
        program->selected_pointer++;
    });

Structure P_POINTER("P_POINTER", [](size_t __index, const char *__src) -> STR_DATA
                    {
    if(__src[__index] == '\\'){
        return {__index, __index, &P_POINTER};
    }else{
        return {};
    } }, [](Program *program, STR_DATA *str){
        if(program->selected_pointer <= 0){
            line_addr adder = __line_adder->get_line(str->start + 1);
            __tb.raise(PointerUnderflow, string_format("out of range at %i:%i", adder.line, adder.offset));
            return;
        }
        program->selected_pointer--;
    });

Structure END_LOOP("END_LOOP", [](size_t __index, const char *__src) -> STR_DATA
                   {
    if(__src[__index] == ']'){
        return {__index, __index, &END_LOOP, true};
    }else{
        return {};
    } }, [](Program *program, STR_DATA *str){});

Structure END_POINTER_LOOP("END_POINTER_LOOP", [](size_t __index, const char *__src) -> STR_DATA
                           {
    if(__src[__index] == '}'){
        return {__index, __index, &END_POINTER_LOOP, true};
    }else{
        return {};
    } }, [](Program *program, STR_DATA *str){});

Structure LOOP("LOOP", [](size_t __index, const char *__src) -> STR_DATA
               {
    if(__src[__index] == '['){
        size_t _start = __index, _end;
        bool is_ended = false;
        line_addr adder;

        STR_DATA res = {_start};
        res.type = &LOOP;
        res.inner = Structure::detector(__index+1, strlen(__src), __src, [&](STR_DATA *__str) -> bool{
            if(__str->type == &END_LOOP){
                _end = __str->end;
                is_ended = true;
                return true;
            }else if(__str->type == &END_POINTER_LOOP){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("'}' without '{' at %i:%i", adder.line, adder.offset));
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
    } }, [](Program *program, STR_DATA *str){
        STR_DATA aloc;
        while(program->memory[program->pointers[program->selected_pointer]]){
            for(size_t i = 0; i < str->inner.size(); i++){
                aloc = str->inner[i];
                str->inner[i].type->run(program, &aloc);
            }
        }
    });

Structure POINTER_LOOP("POINTER_LOOP", [](size_t __index, const char *__src) -> STR_DATA
                       {
    if(__src[__index] == '{'){
        size_t _start = __index, _end;
        bool is_ended = false;
        line_addr adder;

        STR_DATA res = {_start};
        res.type = &POINTER_LOOP;
        res.inner = Structure::detector(__index+1, strlen(__src), __src, [&](STR_DATA *__str) -> bool{
            if(__str->type == &END_LOOP){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("']' without '[' at %i:%i", adder.line, adder.offset));
                return false;
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
    } }, [](Program *program, STR_DATA *str){
        STR_DATA aloc;
        while(program->pointers[program->selected_pointer]){
            for(size_t i = 0; i < str->inner.size(); i++){
                aloc = str->inner[i];
                str->inner[i].type->run(program, &aloc);
            }
        }
    });

Structure::Structure(const char *__src)
{
    __line_adder = new LineAddr((char *)__src);
    line_addr adder;
    this->tree = this->detector(0, strlen(__src), __src, [&](STR_DATA *__str) -> bool
                                {
            if(__str->type == &END_LOOP){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("'}' without '{' at %i:%i", adder.line, adder.offset));
            }else if(__str->type == &END_POINTER_LOOP){
                adder = __line_adder->get_line(__str->start+1);
                __tb.raise(SyntaxError, string_format("'}' without '{' at %i:%i", adder.line, adder.offset));
            }
            return false; });
}

int main(int argc, char const *argv[])
{
    return 0;
}
