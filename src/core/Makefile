CPPFILES = 	Breakpoint.cpp BreakpointTable.cpp DebuggingInformationEntry.cpp \
			DIECompileUnit.cpp DIESubprogram.cpp DIEFormalParameter.cpp \
			CUHeader.cpp DebugInfo.cpp DebugLine.cpp DebugAddressRanges.cpp \
			DwarfDebug.cpp vdb.cpp

CC = 		gcc

CFLAGS =	-shared -fPIC -Wl,-whole-archive libdwarf.a -Wl,-no-whole-archive \
            -lelf -lunwind-ptrace -lunwind-generic -D ENV64 -lstdc++ \
			-std=c++14 -gdwarf-5


all: $(CPPFILES)
	$(CC) -o libvdb.so $^ $(CFLAGS)

clean:
	rm libvdb.so
