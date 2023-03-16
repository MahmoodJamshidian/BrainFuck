#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <functional>
#include <string>
#include <fstream>

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
class Function;

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
using signal_func = std::function<void()>;

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

class Function: public Environment
{
    public:
    Environment *env;
    explicit Function(std::vector<STR_DATA> main_struct, Environment *env) : Environment(main_struct)
    {
        this->env = env;
    }
};

char *__src;

Structure ADD("ADD", [](size_t __index, const char *__src) -> STR_DATA
              {
    if(__src[__index] == '+'){
        return {__index, __index, &ADD};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        env->memory[env->pointers[env->selected_pointer]]++;
    });

Structure SUB("SUB", [](size_t __index, const char *__src) -> STR_DATA
              {
    if(__src[__index] == '-'){
        return {__index, __index, &SUB};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        env->memory[env->pointers[env->selected_pointer]]--;
    });

Structure LFT("LFT", [](size_t __index, const char *__src) -> STR_DATA
              {
    if(__src[__index] == '<'){
        return {__index, __index, &LFT};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        if(env->pointers[env->selected_pointer] <= 0){
            line_addr adder = __line_adder->get_line(str->start + 1);
            __tb.raise(MemoryUnderflow, string_format("out of range at %i:%i", adder.line, adder.offset));
            return;
        }
        env->pointers[env->selected_pointer]--;
    });

Structure RGT("RGT", [](size_t __index, const char *__src) -> STR_DATA
              {
    if(__src[__index] == '>'){
        return {__index, __index, &RGT};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        env->pointers[env->selected_pointer]++;
        while(env->pointers[env->selected_pointer] >= env->memory.size()){
            env->memory.push_back(0);
        }
    });

Structure INP("INP", [](size_t __index, const char *__src) -> STR_DATA
              {
    if(__src[__index] == ','){
        return {__index, __index, &INP};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        env->memory[env->pointers[env->selected_pointer]] = readKey();
    });

Structure OUT("OUT", [](size_t __index, const char *__src) -> STR_DATA
              {
    if(__src[__index] == '.'){
        return {__index, __index, &OUT};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        std::cout << env->memory[env->pointers[env->selected_pointer]];
    });

Structure N_POINTER("N_POINTER", [](size_t __index, const char *__src) -> STR_DATA
                    {
    if(__src[__index] == '/'){
        return {__index, __index, &N_POINTER};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        env->selected_pointer++;
        while(env->selected_pointer >= env->pointers.size()){
            env->pointers.push_back(env->pointers[env->selected_pointer-1]);
        }
    });

Structure P_POINTER("P_POINTER", [](size_t __index, const char *__src) -> STR_DATA
                    {
    if(__src[__index] == '\\'){
        return {__index, __index, &P_POINTER};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        if(env->selected_pointer <= 0){
            line_addr adder = __line_adder->get_line(str->start + 1);
            __tb.raise(PointerUnderflow, string_format("out of range at %i:%i", adder.line, adder.offset));
            return;
        }
        env->selected_pointer--;
    });

Structure END_LOOP("END_LOOP", [](size_t __index, const char *__src) -> STR_DATA
                   {
    if(__src[__index] == ']'){
        return {__index, __index, &END_LOOP, true};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){});

Structure END_POINTER_LOOP("END_POINTER_LOOP", [](size_t __index, const char *__src) -> STR_DATA
                           {
    if(__src[__index] == '}'){
        return {__index, __index, &END_POINTER_LOOP, true};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){});

Structure END_PART("END_PART", [](size_t __index, const char *__src) -> STR_DATA
                   {
    if(__src[__index] == ')'){
        return {__index, __index, &END_PART, true};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){});

Structure LOOP("LOOP", [](size_t __index, const char *__src) -> STR_DATA
               {
    if(__src[__index] == '['){
        size_t _start = __index, _end;
        bool is_ended = false;
        line_addr adder;

        STR_DATA res = {_start};
        res.type = &LOOP;
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
    });

Structure RET("RET", [](size_t __index, const char *__src) -> STR_DATA
                   {
    if(__src[__index] == '&'){
        return {__index, __index, &RET};
    }else{
        return {};
    } }, [](Environment *env, STR_DATA *str){
        env->signal(0);
    });

Structure PART("PART", [](size_t __index, const char *__src) -> STR_DATA
                   {
    if(__src[__index] == '('){
        size_t _start = __index, _end;
        bool is_ended = false;
        line_addr adder;

        STR_DATA res = {_start};
        res.type = &PART;
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

Structure END_FUNC("END_FUNC", [](size_t __index, const char *__src) -> STR_DATA
                   {
    return {};
    }, [](Environment *env, STR_DATA *str){});

Structure FUNC("FUNC", [](size_t __index, const char *__src) -> STR_DATA
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
        res.type = &FUNC;
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
        Environment func(str->inner);
        func.functions = env->functions;
        env->functions.push_back(func);
        env->memory[env->pointers[env->selected_pointer]] = env->functions.size() - 1;
    });

Structure END_CALL_FUNC("END_CALL_FUNC", [](size_t __index, const char *__src) -> STR_DATA
                   {
    return {};
    }, [](Environment *env, STR_DATA *str){});

Structure CALL_FUNC("CALL_FUNC", [](size_t __index, const char *__src) -> STR_DATA
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
        res.type = &CALL_FUNC;
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
            }else if(__str->type == &FUNC){
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

Structure BREAK_POINT("BREAK_POINT", [](size_t __index, const char *__src) -> STR_DATA
                   {
    if(__src[__index] == '|'){

    }
    }, [](Environment *env, STR_DATA *str){});

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
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        std::string app_name = argv[0];
        app_name = app_name.substr(app_name.find_last_of("/") + 1);
        std::cout << "Usage: " << app_name << " <file>" << std::endl;
        return Exception;
    }
    char const *filename = argv[1];
    const char *ext = std::string(filename).substr(std::string(filename).find_last_of(".") + 1).c_str();
    if (strcmp(ext, "bf") != 0 && strcmp(ext, "b") != 0)
    {
        __tb.raise(Exception, "invalid file extension");
        return Exception;
    }
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
    Environment program(main_struct.tree);
    program.run();
    return 0;
}
