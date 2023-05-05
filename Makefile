CPPC=g++
CPPC_ARGS=
CPYC=cython
CPYC_ARGS=-3 --cplus
MAIN_NAME=brainfuck
DLL_NAME=libbfx
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
	$(CPPC) $(MAIN_NAME).cpp -o $(EXE_FILE) $(CPPC_ARGS)
compile-pyx:
	mkdir -p build
	$(CPYC) $(CPYC_ARGS) -o build/$(DLL_NAME).cpp $(DLL_NAME).pyx
	$(CPPC) -c -fPIC -I$(INCLUDE_PATH) -I. -o build/$(DLL_NAME).o build/$(DLL_NAME).cpp
	$(CPPC) -shared -L$(LIB_PATH) -o libbfx.so build/$(DLL_NAME).o -l$(LIB_NAME)
