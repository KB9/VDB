#include "vdb.hpp"

#include <sys/ptrace.h>
#include <stdarg.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <sys/stat.h>
#include <stdio.h>

#include "dwarf/DwarfDebug.hpp"
#include <cstring>

// TODO: Move this function to its own dedicated file
void procmsg(const char* format, ...)
{
    va_list ap;
    fprintf(stdout, "[%d] ", getpid());
    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);
}

// TODO: Move this function to its own dedicated file
unsigned getChildInstructionPointer(pid_t target_pid)
{
	user_regs_struct regs;
	ptrace(PTRACE_GETREGS, target_pid, 0, &regs);
#if defined ENV64
	return regs.rip;
#elif defined ENV32
	return regs.eip;
#endif
}

VDB::VDB()
{

}

VDB::~VDB()
{

}

bool VDB::init(const char *executable_name)
{
	if (!isExecutableFile(executable_name))
		return false;

	// Create the DWARF debug data for this target executable
	debug_info = DebugInfo::readFrom(executable_name);

	// Create the debug engine for debugging the target executable
	engine = std::make_shared<DebugEngine>(executable_name, debug_info);

	is_initialized = true;

	return true;
}

std::shared_ptr<DebugInfo> VDB::getDebugInfo()
{
	return debug_info;
}

std::shared_ptr<DebugEngine> VDB::getDebugEngine()
{
	return engine;
}

bool VDB::isInitialized()
{
	return is_initialized;
}

bool VDB::isExecutableFile(const char *executable_name)
{
	struct stat st;
	if (stat(executable_name, &st) < 0)
		return false;
	if ((st.st_mode & S_IEXEC) != 0)
		return true;
	return false;
}