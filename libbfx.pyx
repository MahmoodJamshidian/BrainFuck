from libcpp.vector cimport vector
from libcpp.functional cimport function
from libcpp.string cimport string
from libc.stdlib cimport malloc
from libc.stdint cimport *
from libcpp cimport bool
import traceback

cdef extern from "libbfx.cpp":
    cdef void g_writeKey(char _val)
    cdef char g_readKey()
    ctypedef struct registry:
        vector[uint8_t] nreg
    cdef struct STR_DATA:
        size_t start, end
        Structure *type
        bool ignore
        vector[STR_DATA] inner
    cdef registry reg
    ctypedef bool (*handler_t)(STR_DATA *)
    ctypedef STR_DATA(*check_func)(size_t, const char *)
    ctypedef void(*react_func)(Environment *, STR_DATA *)
    ctypedef string(*build_func)(STR_DATA *)
    ctypedef void(*signal_func)()
    cdef cppclass Structure:
        vector[STR_DATA] tree
        Structure(const char *__src)
        Structure(const char *__name, check_func *__checker, build_func __builder, react_func __reacter)
        vector[STR_DATA] detector(size_t __index, size_t __eof, const char *__src, handler_t __handler)
        void run(Environment *env, STR_DATA *str)
        string build(STR_DATA *str)
    cdef cppclass iostream:
        void read(char &*_val, size_t _size)
        size_t write(char &*_val, size_t _size)
    cdef cppclass Environment:
        registry reg
        vector[uint8_t] memory
        vector[size_t] pointers
        vector[Environment] functions
        size_t selected_pointer
        function[char] readKey
        function[void] writeKey
        iostream *_stream
        function[void] _on_read, _on_write
        Environment(vector[STR_DATA] structs, iostream *_stream, signal_func _on_read, signal_func _on_write)
        void add_signal(signal_func handler)
        void signal(uint8_t sig)
        void run()
        string build()
    cdef Structure S_ADD, S_SUB, S_LFT, S_RGT, S_INP, S_OUT, S_N_POINTER, S_P_POINTER, S_LOOP, S_POINTER_LOOP, S_RET, S_PART, S_FUNC, S_CALL_FUNC
    cdef cppclass Program(Environment):
        Program(Structure *main_struct, iostream *_stream, signal_func _on_read, signal_func _on_write)
    cdef void load_plugin(const char *dir, bool overwrite)

cdef class bfx_STR_DATA:
    cdef STR_DATA *_str
    cdef void set(self, STR_DATA *_str):
        self._str = _str
    cdef STR_DATA* get(self):
        return self._str

cdef class bfx_Environment:
    cdef Environment *_env
    cdef void set(self, Environment *_env):
        self._env = _env
    cdef Environment *get(self):
        return self._env

cdef class bfx_Structure:
    cdef Structure *_structure
    cdef vector[STR_DATA] get_tree(self):
        return self._structure.tree
    def __init__(self, string __src):
        self._structure = new Structure(__src.c_str())
    def build(self, bfx_STR_DATA _str):
        return self._structure.build(_str.get())
    def run(self, bfx_Environment _env, bfx_STR_DATA _str):
        cdef Environment *_cenv = _env.get()
        cdef STR_DATA *_cstr = _str.get()
        self._structure.run(_cenv, _cstr)

cdef class bfx_iostream:
    cdef iostream *_stream
    def __init__(self):
        self._stream = new iostream()
    def read(self, buffer_size=1024):
        cdef char *res = <char *>malloc(buffer_size)
        self._stream.read(res, buffer_size)
        return res
    def write(self, string data):
        return self._stream.write(data.c_str(), len(data))
    
cdef object _g_on_read, _g_on_write
cdef void _c_on_read():
    try:
        _g_on_read()
    except:
        traceback.print_exc()
        exit(1)
cdef void _c_on_write():
    try:
        _g_on_write()
    except:
        traceback.print_exc()
        exit(1)

cdef class bfx_Program:
    cdef Program *_program
    def __init__(self, bfx_Structure main_struct, bfx_iostream _stream = None, _on_read = lambda: None, _on_write = lambda: None):
        global _g_on_read, _g_on_write
        _g_on_read = _on_read
        _g_on_write = _on_write
        cdef iostream *_stream_obj = NULL
        if _stream:
            _stream_obj = _stream._stream
            
        self._program = new Program(main_struct._structure, _stream=_stream_obj, _on_read=<signal_func>_c_on_read, _on_write=<signal_func>_c_on_write)
    def run(self):
        self._program.run()
    def build(self):
        return self._program.build()

cpdef void bfx_load_plugin(string dir, bool overwrite = False):
    load_plugin(dir.c_str(), overwrite)
