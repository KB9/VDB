#include "vdb.hpp"

#include <thread>
#include <sys/ptrace.h>
#include <stdarg.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <stdio.h>

#include "DwarfDebug.hpp"

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
unsigned getChildInstructionPointer(pid_t child_pid)
{
	user_regs_struct regs;
	ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
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

bool VDB::run(const char *executable_name)
{
	pid_t child_pid = fork();
	if (child_pid == 0)
	{
		runTarget(executable_name);
	}
	else if (child_pid > 0)
	{
		runDebugger(child_pid, executable_name);
	}
	else
	{
		perror("fork");
		return false;
	}

	return true;
}

bool VDB::runTarget(const char *executable_name)
{
	procmsg("Target started. Will run '%s'\n", executable_name);

	// Allow tracing of this process
	if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0)
	{
		perror("ptrace");
		return false;
	}

	// Replace this process's image with the given program
	execl(executable_name, executable_name, 0);

	return false;
}

bool VDB::runDebugger(pid_t child_pid, const char *child_name)
{
	DwarfDebug dwarf(child_name);

	return true;
}