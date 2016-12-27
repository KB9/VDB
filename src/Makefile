CPPFILES = Breakpoint.cpp DwarfCpp.cpp main.cpp
CC = gcc

run: all
	./vdb ../../../testchild

all: $(CPPFILES)
	$(CC) -o vdb $^ -ldwarf -lelf -D ENV64 -gdwarf -lstdc++ -std=c++14

clean:
	rm vdb
