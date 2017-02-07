CPPFILES = 	Breakpoint.cpp BreakpointTable.cpp DebuggingInformationEntry.cpp \
			DIECompileUnit.cpp DIESubprogram.cpp DIEFormalParameter.cpp \
			CUHeader.cpp DebugInfo.cpp DebugLine.cpp DebugAddressRanges.cpp \
			DwarfDebug.cpp main.cpp

CC = 		gcc

CFLAGS =	-ldwarf -lelf -lunwind-ptrace -lunwind-generic -D ENV64 -lstdc++ \
			-std=c++14 -gdwarf-5

run: all
	./vdb ../../roar #../../../testchild

all: $(CPPFILES)
	$(CC) -o vdb $^ $(CFLAGS)

clean:
	rm vdb
