CPPFILES = Breakpoint.cpp BreakpointTable.cpp DebuggingInformationEntry.cpp DIECompileUnit.cpp DIESubprogram.cpp CUHeader.cpp DebugInfo.cpp DebugLine.cpp DwarfDebug.cpp main.cpp
CC = gcc

run: all
	./vdb ../../../testchild

all: $(CPPFILES)
	$(CC) -o vdb $^ -ldwarf -lelf -D ENV64 -gdwarf -lstdc++ -std=c++14

clean:
	rm vdb
