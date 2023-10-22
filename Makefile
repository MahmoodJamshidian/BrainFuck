CPPC=g++
CPPC_ARGS=
CPYC=cython
CPYC_ARGS=-3 --cplus
MAIN_NAME=brainfuck
DLL_NAME=libbfx
BUILD_NAME_SUFIX=_build
INCLUDE_PATH=/usr/include/python3.11
LIB_PATH=/usr/lib/python3.11
LIB_NAME=python3.11
ifeq ($(OS),Windows_NT)
EXE_FILE=$(MAIN_NAME).exe
DLL_FILE=$(DLL_NAME).pyd
else
EXE_FILE=$(MAIN_NAME)
DLL_FILE=$(DLL_NAME).so
endif

compile:
	mkdir -p dist
	$(CPPC) $(MAIN_NAME).cpp -I. -o dist/$(EXE_FILE) $(CPPC_ARGS)
compile-pyx:
	mkdir -p build
	$(CPYC) $(CPYC_ARGS) -o build/$(DLL_NAME)$(BUILD_NAME_SUFIX).cpp $(DLL_NAME).pyx
	$(CPPC) -c -fPIC -I$(INCLUDE_PATH) -I. -o build/$(DLL_NAME)$(BUILD_NAME_SUFIX).o build/$(DLL_NAME)$(BUILD_NAME_SUFIX).cpp
	$(CPPC) -shared -L$(LIB_PATH) -I. -o dist/$(DLL_FILE) build/$(DLL_NAME)$(BUILD_NAME_SUFIX).o -l$(LIB_NAME)
