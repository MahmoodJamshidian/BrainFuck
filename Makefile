CC=g++
ARGS=
MAIN_FILE=brainfuck
ifeq ($(OS),Windows_NT)
EXTENITION_EXE_FILE=.exe
else
EXTENITION_EXE_FILE=
endif

compile:
	$(CC) $(MAIN_FILE).cpp -o $(MAIN_FILE)$(EXTENITION_EXE_FILE) $(ARGS)