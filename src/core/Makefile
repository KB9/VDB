CPPFILES = 	Breakpoint.cpp \
			BreakpointTable.cpp \
			Unwinder.cpp \
			dwarf/DwarfReader.cpp \
			dwarf/DebugLine.cpp \
			dwarf/DebugAddressRanges.cpp \
			dwarf/DwarfDebug.cpp \
			dwarf/DwarfExprInterpreter.cpp \
			dwarf/ValueDeducer.cpp \
			DebugInfo.cpp \
			StepCursor.cpp \
			ProcessDebugger.cpp \
			DebugEngine.cpp \
			vdb.cpp

CC = 		gcc

CFLAGS =	-shared -fPIC -Wl,-whole-archive libdwarf.a -Wl,-no-whole-archive \
            -lelf -lunwind-ptrace -lunwind-generic -D ENV64 -lstdc++ \
			-std=c++14 -gdwarf-5 -Wall -Wpedantic -I/usr/include/libdwarf


all: $(CPPFILES)
	$(CC) -o libvdb.so $^ $(CFLAGS)

clean:
	rm libvdb.so
