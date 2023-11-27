#include "libbfx.hpp"
#include <iostream>
#include "path.cpp"

#ifndef BFX
#define BFX

void g_writeKey(char _val)
{
    std::cout << _val;
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
char g_readKey()
{
    return (char)_getch();
}

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
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

registry_value::registry_value(uint64_t *reg_ptr) : reg_ptr(reg_ptr) {}

uint64_t registry_value::operator=(uint64_t val)
{
    return (*(this->reg_ptr) = val);
}

uint64_t registry_value::operator++()
{
    return (++ (*this->reg_ptr));
}

uint64_t registry_value::operator--()
{
    return (-- (*this->reg_ptr));
}

uint64_t registry_value::operator++(int)
{
    return (++ (*this->reg_ptr));
}

uint64_t registry_value::operator--(int)
{
    return (-- (*this->reg_ptr));
}

uint64_t registry_value::operator+=(uint64_t val)
{
    return (*(this->reg_ptr) += val);
}

uint64_t registry_value::operator-=(uint64_t val)
{
    return (*(this->reg_ptr) -= val);
}

registry_value::operator bool()
{
    return (bool)(*this->reg_ptr);
}

bool registry_value::operator!()
{
    return !(*(this->reg_ptr));
}

registry::registry()
{
    this->mem = (uint64_t *)malloc(sizeof(uint64_t));
    this->mem_size = 1;
}

registry_value registry::operator[](uint64_t ind)
{
    if (ind >= this->mem_size)
    {
        this->resize(ind+1);
    }

    return registry_value(this->mem + ind);
}

uint64_t registry::get(uint64_t ind)
{
    if (ind < this->mem_size)
    {
        return *(this->mem + ind);
    }
    else
    {
        this->resize(ind+1);
        return 0;
    }
}

void registry::set(uint64_t ind, uint64_t val)
{
    if (ind >= this->mem_size)
    {
        this->resize(ind+1);
    }

    *(this->mem + ind) = val;
}

void registry::resize(uint64_t size)
{
    if (size < this->mem_size)
    {
        throw std::runtime_error("new size is invalid");
    }
    else
    {
        this->mem = (uint64_t *)realloc(this->mem, sizeof(uint64_t) * size);
        this->mem_size = size;
    }
}

registry::~registry()
{
    free(this->mem);
}


std::vector<Structure *> structers;

Structure *get_structure(const char *name)
{
    for (uint64_t ind = 0; ind < structers.size(); ind++)
    {
        if (!strcmp(structers[ind]->name, name))
        {
            return structers[ind];
        }
    }

    return NULL;
}

LineAddr::LineAddr(char *msg) : __msg(msg) {}

line_addr LineAddr::get_line(size_t __offset)
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

std::vector<STR_DATA> Structure::detector(size_t __index, size_t __eof, const char *__src, detect_func __handler = NULL)
{
    std::vector<STR_DATA> tree;
    bool is_handled = false;
    STR_DATA res;
    bool closed = false;
    while (__index < __eof)
    {
        for (uint8_t i = 0; i < structers.size(); i++)
        {
            res = structers.at(i)->__checker(__index, __src);
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

Structure::Structure(const char *__name, check_func __checker, build_func __builder, react_func __reacter) : name(__name)
{
    this->__checker = __checker;
    this->__reacter = __reacter;
    this->__builder = __builder;
    structers.push_back(this);
}

void Structure::run(Environment *env, STR_DATA *str)
{
    this->__reacter(env, str);
}

std::string Structure::build(STR_DATA *str)
{
    return this->__builder(str);
}


bool iostream::check_is_allow_to_read()
{
    if (_inp.length() > 0)
        return true;
    return false;
}

size_t iostream::operator<<(std::string _val)
{
    _inp += _val;
    return _val.length();
}

size_t iostream::operator<<(char *_val)
{
    this->_inp += _val;
    return strlen(_val);
}

size_t iostream::operator<<(char _val)
{
    this->_inp += _val;
    return 1;
}

size_t iostream::operator>>(std::string &_val)
{
    _val = this->_out;
    this->_out = "";
    return _val.length();
}

size_t iostream::operator>>(char &_val)
{
    if (this->_out.length() > 0)
    {
        _val = *(char *)_out.substr(0, 1).c_str();
        this->_out.erase(0, 1);
        return 1;
    }
    return 0;
}

void iostream::read(char &_val)
{
    this->operator>>(_val);
}

void iostream::read(char *&_val, size_t _size)
{
    _val = (char *)this->_out.substr(0, _size).c_str();
    this->_out.erase(0, _size);
}

void iostream::read(std::string &_val, size_t _size)
{
    _val = this->_out.substr(0, _size);
    this->_out.erase(0, _size);
}

size_t iostream::write(char _val)
{
    this->_inp += _val;
    return 1;
}

size_t iostream::write(char *_val, size_t _size)
{
    for (size_t _ind = 0; _ind < _size; _ind++)
    {
        this->_inp += _val[_ind];
    }
    return _size;
}

size_t iostream::write(std::string _val, size_t _size)
{
    for (size_t _ind = 0; _ind < _size; _ind++)
    {
        this->_inp += _val[_ind];
    }
    return _size;
}

std::string read_iostream(iostream *_stream)
{
    while (_stream->check_is_allow_to_read()){}
    std::string res = _stream->_inp;
    _stream->_inp = "";
    return res;
}

std::string read_iostream(iostream *_stream, size_t _size)
{
    while (!_stream->check_is_allow_to_read()){}
    std::string res = _stream->_inp.substr(0, _size);
    _stream->_inp.erase(0, _size);
    return res;
}

size_t write_iostream(iostream *_stream, char _val)
{
    _stream->_out += _val;
    return 1;
}

size_t write_iostream(iostream *_stream, char *_val, size_t _size)
{
    for (size_t _ind = 0; _ind < _size; _ind++)
    {
        _stream->_out += _val[_ind];
    }
    return _size;
}

size_t write_iostream(iostream *_stream, std::string _val)
{
    _stream->_out += _val;
    return _val.length();
}

Environment::Environment(std::vector<STR_DATA> structs, iostream *_stream = NULL, std::function<void()> _on_read = [](){}, std::function<void()> _on_write = [](){})
{
    this->structs = structs;
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
            write_iostream(this->_stream, _val);
            if (_on_read != NULL)
                this->_on_write();
        };
    }
}

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

std::string Environment::build()
{
    std::string res;
    for (size_t _ind = 0; _ind < this->structs.size(); _ind++)
    {
        res += this->structs[_ind].type->build(&this->structs[_ind]);
        res += ',';
    }
    res.pop_back();
    return res;
}

Traceback __tb(stderr);

registry reg;

LineAddr *__line_adder;

std::vector<STR_DATA> src;

char *__source;

Structure S_ADD("ADD", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == '+')
        {
            return {__index, __index, &S_ADD};
        }else{
            return {};
        }
    }, [](STR_DATA *str) -> std::string
    {
        return "{&S_ADD}";
    }, [](Environment *env, STR_DATA *str)
    {
        env->memory[env->pointers[env->selected_pointer]]++;
    }
);

Structure S_SUB("SUB", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == '-')
        {
            return {__index, __index, &S_SUB};
        }else{
            return {};
        }
    }, [](STR_DATA *str) -> std::string
    {
        return "{&S_SUB}";
    }, [](Environment *env, STR_DATA *str)
    {
        env->memory[env->pointers[env->selected_pointer]]--;
    }
);

Structure S_LFT("LFT", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == '<')
        {
            return {__index, __index, &S_LFT};
        }else{
            return {};
        }
    }, [](STR_DATA *str) -> std::string
    {
        return "{&S_LFT}";
    }, [](Environment *env, STR_DATA *str)
    {
        if(env->pointers[env->selected_pointer] <= 0)
        {
            line_addr adder = __line_adder->get_line(str->start + 1);
            __tb.raise(MemoryUnderflow, string_format("out of range at %i:%i", adder.line, adder.offset));
            return;
        }
        env->pointers[env->selected_pointer]--;
    }
);

Structure S_RGT("RGT", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == '>')
        {
            return {__index, __index, &S_RGT};
        }else{
            return {};
        }
    }, [](STR_DATA *str) -> std::string
    {
        return "{&S_RGT}";
    }, [](Environment *env, STR_DATA *str)
    {
        env->pointers[env->selected_pointer]++;
        while(env->pointers[env->selected_pointer] >= env->memory.size())
        {
            env->memory.push_back(0);
        }
    }
);

Structure S_INP("INP", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == ',')
        {
            return {__index, __index, &S_INP};
        }else{
            return {};
        }
    }, [](STR_DATA *str) -> std::string
    {
        return "{&S_INP}";
    }, [](Environment *env, STR_DATA *str)
    {
        env->memory[env->pointers[env->selected_pointer]] = env->readKey();
    }
);

Structure S_OUT("OUT", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == '.')
        {
            return {__index, __index, &S_OUT};
        }else{
            return {};
        }
    }, [](STR_DATA *str) -> std::string
    {
        return "{&S_OUT}";
    }, [](Environment *env, STR_DATA *str)
    {
        env->writeKey(env->memory[env->pointers[env->selected_pointer]]);
    }
);

Structure S_N_POINTER("N_POINTER", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == '/')
        {
            return {__index, __index, &S_N_POINTER};
        }else{
            return {};
        }
    }, [](STR_DATA *str) -> std::string
    {
        return "{&S_N_POINTER}";
    }, [](Environment *env, STR_DATA *str)
    {
        env->selected_pointer++;
        while(env->selected_pointer >= env->pointers.size())
        {
            env->pointers.push_back(env->pointers[env->selected_pointer-1]);
        }
    }
);

Structure S_P_POINTER("P_POINTER", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == '\\')
        {
            return {__index, __index, &S_P_POINTER};
        }else{
            return {};
        }
    }, [](STR_DATA *str) -> std::string
    {
        return "{&S_P_POINTER}";
    }, [](Environment *env, STR_DATA *str)
    {
        if(env->selected_pointer <= 0)
        {
            line_addr adder = __line_adder->get_line(str->start + 1);
            __tb.raise(PointerUnderflow, string_format("out of range at %i:%i", adder.line, adder.offset));
            return;
        }
        env->selected_pointer--;
    }
);

Structure S_END_LOOP("END_LOOP", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == ']')
        {
            if(!reg[0])
            {
                line_addr adder;
                adder = __line_adder->get_line(__index);
                __tb.raise(SyntaxError, string_format("']' without '[' at %i:%i", adder.line, adder.offset));
            }
            reg[0] --;
            return {__index, __index, &S_END_LOOP, true};
        }else{
            return {};
        }
    }, NULL, NULL
);

Structure S_END_POINTER_LOOP("END_POINTER_LOOP", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == '}')
        {
            if(!reg[1])
            {
                line_addr adder;
                adder = __line_adder->get_line(__index);
                __tb.raise(SyntaxError, string_format("'}' without '{' at %i:%i", adder.line, adder.offset));
            }
            reg[1] --;
            return {__index, __index, &S_END_POINTER_LOOP, true};
        }else{
            return {};
        }
    }, NULL, NULL
);

Structure S_END_PART("END_PART", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == ')')
        {
            if(!reg[2])
            {
                line_addr adder;
                adder = __line_adder->get_line(__index);
                __tb.raise(SyntaxError, string_format("')' without '(' at %i:%i", adder.line, adder.offset));
            }
            reg[2] --;
            return {__index, __index, &S_END_PART, true};
        }else{
            return {};
        }
    }, NULL, NULL
);

Structure S_LOOP("LOOP", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == '[')
        {
            reg[0] ++;
            size_t _start = __index, _end;
            bool is_ended = false;
            line_addr adder;

            STR_DATA res = {_start};
            res.type = &S_LOOP;
            res.inner = Structure::detector(__index+1, strlen(__src), __src, [&](STR_DATA *__str) -> bool{
                if(__str->type == &S_END_LOOP)
                {
                    _end = __str->end;
                    is_ended = true;
                    return true;
                }
                return false;
            });
            if(!is_ended)
            {
                adder = __line_adder->get_line(__index+1);
                __tb.raise(SyntaxError, string_format("Missing ']' at %i:%i", adder.line, adder.offset));
                return {};
            }
            res.end = _end;
            return res;

        }else{
            return {};
        }
    }, [](STR_DATA *str) -> std::string
    {
        std::string res = "{&S_LOOP,{";
        for(size_t i = 0; i < str->inner.size(); i++)
        {
            res += str->inner[i].type->build(&str->inner[i]) + ",";
        }
        res += "}}";
        return res;
    }, [](Environment *env, STR_DATA *str)
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

Structure S_POINTER_LOOP("POINTER_LOOP", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == '{')
        {
            reg[1] ++;
            size_t _start = __index, _end;
            bool is_ended = false;
            line_addr adder;

            STR_DATA res = {_start};
            res.type = &S_POINTER_LOOP;
            res.inner = Structure::detector(__index+1, strlen(__src), __src, [&](STR_DATA *__str) -> bool
                {
                    if(__str->type == &S_END_POINTER_LOOP)
                    {
                        _end = __str->end;
                        is_ended = true;
                        return true;
                    }
                    return false;
                }
            );
            if(!is_ended)
            {
                adder = __line_adder->get_line(__index+1);
                __tb.raise(SyntaxError, string_format("Missing '}' at %i:%i", adder.line, adder.offset));
                return {};
            }
            res.end = _end;
            return res;
        }else{
            return {};
        }
    }, [](STR_DATA *str) -> std::string
    {
        std::string res = "{&S_POINTER_LOOP,{";
        for(size_t i = 0; i < str->inner.size(); i++)
        {
            res += str->inner[i].type->build(&str->inner[i]) + ",";
        }
        res += "}}";
        return res;
    }, [](Environment *env, STR_DATA *str)
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

Structure S_RET("RET", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == '&')
        {
            return {__index, __index, &S_RET};
        }else{
            return {};
        }
    }, [](STR_DATA *str) -> std::string
    {
        return "{&S_RET}";
    }, [](Environment *env, STR_DATA *str)
    {
        env->signal(0);
    }
);

Structure S_PART("PART", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == '(')
        {
            reg[2] ++;
            size_t _start = __index, _end;
            bool is_ended = false;
            line_addr adder;

            STR_DATA res = {_start};
            res.type = &S_PART;
            res.inner = Structure::detector(__index+1, strlen(__src), __src, [&](STR_DATA *__str) -> bool
            {
                if(__str->type == &S_END_PART)
                {
                    _end = __str->end;
                    is_ended = true;
                    return true;
                }
                return false;
            });
            if(!is_ended)
            {
                adder = __line_adder->get_line(__index+1);
                __tb.raise(SyntaxError, string_format("Missing ')' at %i:%i", adder.line, adder.offset));
                return {};
            }
            res.end = _end;
            return res;
        }else{
            return {};
        }
    }, [](STR_DATA *str) -> std::string
    {
        std::string res = "{&S_PART,{";
        for(size_t i = 0; i < str->inner.size(); i++)
        {
            res += str->inner[i].type->build(&str->inner[i]) + ",";
        }
        res += "}}";
        return res;
    }, [](Environment *env, STR_DATA *str)
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
        venv.readKey = env->readKey;
        venv.writeKey = env->writeKey;
        venv.run();
    });

Structure S_END_FUNC("END_FUNC", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == '%')
        {
            if(reg[3])
            {
                return {__index, __index, &S_END_FUNC, true};
            }else{
                return {};
            }
        }else{
            return {};
        }
    }, NULL, NULL
);

Structure S_FUNC("FUNC", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == '%')
        {
            reg[3] = 1;
            size_t _start = __index, _end;
            bool is_ended = false;
            line_addr adder;

            STR_DATA res = {_start};
            res.type = &S_FUNC;
            res.inner = Structure::detector(__index+1, strlen(__src), __src, [&](STR_DATA *__str) -> bool{
                if(__str->type == &S_END_FUNC)
                {
                    _end = __str->end;
                    is_ended = true;
                    return true;
                }
                return false;
            });
            if(!is_ended)
            {
                adder = __line_adder->get_line(__index+1);
                __tb.raise(SyntaxError, string_format("Function not closed at %i:%i", adder.line, adder.offset));
                return {};
            }
            res.end = _end;
            reg[3] = 0;
            return res;
        }else{
            return {};
        }
    }, [](STR_DATA *str) -> std::string
    {
        std::string res = "{&S_FUNC,{";
        for(size_t i = 0; i < str->inner.size(); i++)
        {
            res += str->inner[i].type->build(&str->inner[i]) + ",";
        }
        res += "}}";
        return res;
    }, [](Environment *env, STR_DATA *str)
    {
        Environment func(str->inner);
        func.readKey = env->readKey;
        func.writeKey = env->writeKey;
        func.functions = env->functions;
        env->functions.push_back(func);
        env->memory[env->pointers[env->selected_pointer]] = env->functions.size() - 1;
    }
);

Structure S_END_CALL_FUNC("END_CALL_FUNC", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == '$')
        {
            if(reg[4])
            {
                return {__index, __index, &S_END_CALL_FUNC, true};
            }else{
                return {};
            }
        }else{
            return {};
        }
    }, NULL, NULL
);

Structure S_CALL_FUNC("CALL_FUNC", [](size_t __index, const char *__src) -> STR_DATA
    {
        if(__src[__index] == '$')
        {
            reg[4] = 1;
            size_t _start = __index, _end;
            bool is_ended = false;
            line_addr adder;

            STR_DATA res = {_start};
            res.type = &S_CALL_FUNC;
            res.inner = Structure::detector(__index+1, strlen(__src), __src, [&](STR_DATA *__str) -> bool
            {
                if(__str->type == &S_FUNC)
                {
                    adder = __line_adder->get_line(__str->start+1);
                    __tb.raise(SyntaxError, string_format("illegal function definition at %i:%i", adder.line, adder.offset));
                }else if(__str->type == &S_END_CALL_FUNC)
                {
                    _end = __str->end;
                    is_ended = true;
                    return true;
                }
                return false;
            });
            if(!is_ended)
            {
                adder = __line_adder->get_line(__index+1);
                __tb.raise(SyntaxError, string_format("function not closed at %i:%i", adder.line, adder.offset));
                return {};
            }
            res.end = _end;
            reg[4] = 0;
            return res;
        }else{
            return {};
        }
    }, [](STR_DATA *str) -> std::string
    {
        std::string res = "{&S_CALL_FUNC,{";
        for(size_t i = 0; i < str->inner.size(); i++)
        {
            res += str->inner[i].type->build(&str->inner[i]) + ",";
        }
        res += "}}";
        return res;
    }, [](Environment *env, STR_DATA *str)
    {
        if(env->memory[env->pointers[env->selected_pointer]] >= env->functions.size())
        {
            __line_adder = new LineAddr(__source);
            line_addr adder = __line_adder->get_line(str->start);
            __tb.raise(Undefined, string_format("function Undefined at %i:%i", adder.line, adder.offset));
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
        inner.readKey = env->readKey;
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

Structure::Structure(const char *__src)
{
    __source = (char *)__src;
    __line_adder = new LineAddr((char *)__src);
    line_addr adder;
    this->tree = this->detector(0, strlen(__src), __src);
    this->__builder = [&](STR_DATA *str) -> std::string
    {
        std::string src = "#include \"core.cpp\"\nstatic std::vector<STR_DATA> TREE = {";
        for(size_t _ind = 0; _ind < str->inner.size(); _ind++)
        {
            src += str->inner[_ind].type->build(&str->inner[_ind]) + ',';
        }
        src += "};\n#include \"start.hpp\"";
        return src;
    };
    this->__reacter = [](Environment *env, STR_DATA *str)
    {
        for(size_t _ind = 0; _ind < str->inner.size(); _ind++)
        {
            str->inner[_ind].type->run(env, &str->inner[_ind]);
        }
    };
}

Program::Program(Structure *main_struct, iostream *_stream = NULL, std::function<void()> _on_read = [](){}, std::function<void()> _on_write = [](){}) : Environment({{0, 0, main_struct, false, main_struct->tree}}, _stream, _on_read, _on_write){}
#endif
