CPPC=g++
PYTHON=python3
CPYC=$(PYTHON) -m cython

ifeq ($(OS),Windows_NT)
python_lib:=$(shell $(PYTHON) -c "import sysconfig; print((sysconfig.get_paths()['data']+'\\libs').replace(chr(0x5c), chr(0x5c)*2).replace(' ', '\\ '))")
python_lib_name:=$(shell $(PYTHON) -c "import sysconfig; print('python'+sysconfig.get_config_var('py_version_nodot'))")
exe_suffix=.exe
else
python_lib:=$(shell $(PYTHON) -c "import sysconfig; print(sysconfig.get_config_var('BINLIBDEST'))")
python_lib_name:=$(shell $(PYTHON) -c "import sysconfig; print(sysconfig.get_config_var('BLDLIBRARY')[2:])")
exe_suffix=
endif
python_include:=$(shell $(PYTHON) -c "import sysconfig; print(sysconfig.get_config_var('INCLUDEPY').replace(chr(0x5c), chr(0x5c)*2).replace(' ', '\\ '))")
dll_suffix:=so
ifeq ($(OS),Windows_NT)
dll_suffix:=dll
endif

compile: build/brainfuck.o dist_dir
ifeq ($(OS),Windows_NT)
	$(CPPC) build/brainfuck.o -o dist/brainfuck$(exe_suffix)
else
	$(CPPC) build/brainfuck.o -o dist/brainfuck$(exe_suffix)
endif

compile-pyx: build/_libbfx.o dist_dir
ifeq ($(OS),Windows_NT)
	$(CPPC) -shared -L"$(python_lib)" -I. -o dist/libbfx.pyd build/_libbfx.o -l$(python_lib_name)
else
	$(CPPC) -shared -L"$(python_lib)" -I. -o dist/libbfx.so build/_libbfx.o -l$(python_lib_name)
endif

build/brainfuck.o: brainfuck.cpp libbfx.cpp libbfx.hpp build_dir
	$(CPPC) -c -I. -o build/brainfuck.o brainfuck.cpp

build/_libbfx.o: build/_libbfx.cpp libbfx.cpp libbfx.hpp build_dir
ifeq ($(OS),Windows_NT)
	$(CPPC) -c -fPIC -I $(python_include) -I . -DMS_WIN64 -o build/_libbfx.o build/_libbfx.cpp
else
	$(CPPC) -c -fPIC -I $(python_include) -I . -o build/_libbfx.o build/_libbfx.cpp
endif

build/_libbfx.cpp: libbfx.pyx build_dir
	$(CPYC) -3 --cplus -o build/_libbfx.cpp libbfx.pyx

dist_dir:
	mkdir -p dist

build_dir:
	mkdir -p build

build_plugins: plugins/src
	@mkdir -p plugins/bin; \
	cpp_files=`ls plugins/src/ | grep \.cpp$$`; \
	for cpp_file in $$cpp_files; do \
		g++ -I. -Iplugins/src -shared -fPIC "plugins/src/$${cpp_file}" -D BFX_PLUGIN_BUILD -o "plugins/bin/$${cpp_file%.cpp}.$(dll_suffix)"; \
	done