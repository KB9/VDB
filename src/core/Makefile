CPPFILES = 	Breakpoint.cpp \
			BreakpointTable.cpp \
			dwarf/DebuggingInformationEntry.cpp \
			dwarf/DIEBaseType.cpp \
			dwarf/DIECompileUnit.cpp \
			dwarf/DIELexicalBlock.cpp \
			dwarf/DIEPointerType.cpp \
			dwarf/DIESubprogram.cpp \
			dwarf/DIEFormalParameter.cpp \
			dwarf/DIEVariable.cpp \
			dwarf/CUHeader.cpp \
			dwarf/DebugInfo.cpp \
			dwarf/DebugLine.cpp \
			dwarf/DebugAddressRanges.cpp \
			dwarf/DwarfDebug.cpp \
			dwarf/DwarfExprInterpreter.cpp \
			vdb.cpp

CC = 		gcc

CFLAGS =	-shared -fPIC -Wl,-whole-archive libdwarf.a -Wl,-no-whole-archive \
            -lelf -lunwind-ptrace -lunwind-generic -D ENV64 -lstdc++ \
			-std=c++14 -gdwarf-5 -Wall -Wpedantic -I/usr/include/libdwarf


all: $(CPPFILES)
	$(CC) -o libvdb.so $^ $(CFLAGS)

clean:
	rm libvdb.so
