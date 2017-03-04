#include "vdb.hpp"

#include <thread>
#include <sys/ptrace.h>
#include <stdarg.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <stdio.h>

#include "DwarfDebug.hpp"
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
	if (target_name != NULL) delete target_name;
	target_name = NULL;
}

bool VDB::init(const char *executable_name)
{
	// Assign the new target executable name
	if (target_name != NULL) delete target_name;
	target_name = new char[strlen(executable_name) + 1];
	strcpy(target_name, executable_name);

	// Create the DWARF debug data for this target executable
	dwarf = std::make_shared<DwarfDebug>(target_name);

	// Initialize the breakpoint table
	breakpoint_table = std::make_shared<BreakpointTable>(dwarf);

	return true;
}

bool VDB::run()
{
	pid_t child_pid = fork();
	if (child_pid == 0)
	{
		runTarget();
	}
	else if (child_pid > 0)
	{
		target_pid = child_pid;
		runDebugger();
	}
	else
	{
		perror("fork");
		return false;
	}

	return true;
}

std::shared_ptr<DwarfDebug> VDB::getDwarfDebugData()
{
	return dwarf;
}

bool VDB::runTarget()
{
	procmsg("Target started. Will run '%s'\n", target_name);

	// Allow tracing of this process
	if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0)
	{
		perror("ptrace");
		return false;
	}

	// Replace this process's image with the given program
	execl(target_name, target_name, 0);

	return false;
}

bool VDB::runDebugger()
{
	// Wait for child to stop on its first instruction
	wait(0);
	procmsg("Entry point. EIP = 0x%08x\n", getChildInstructionPointer(target_pid));

	breakpoint_table->enableBreakpoints(target_pid);

	int wait_status;

	while (true)
	{
		// Resume execution
		if (ptrace(PTRACE_CONT, target_pid, 0, 0) < 0)
		{
			perror("ptrace");
			return false;
		}
		wait(&wait_status);

		// If the child process exited
		if (WIFEXITED(wait_status))
		{
			// Stop debugging
			break;
		}
		// If the child process was stopped midway through execution
		else if (WIFSTOPPED(wait_status))
		{
			int last_sig = WSTOPSIG(wait_status);

			// If a breakpoint was hit
			if (last_sig == SIGTRAP)
			{
				procmsg("[DEBUG] Breakpoint hit!\n");

				// TODO: Do stuff at this breakpoint

				// DEBUG: Step over the breakpoint
				user_regs_struct regs;
				ptrace(PTRACE_GETREGS, target_pid, 0, &regs);
				procmsg("[DEBUG] Getting breakpoint at address: 0x%08x\n", regs.rip);
				uint64_t breakpoint_address = regs.rip - 1;
				std::unique_ptr<Breakpoint> breakpoint =
					breakpoint_table->getBreakpoint(breakpoint_address);
				if (breakpoint)
				{
					procmsg("[DEBUG] Stepping over breakpoint at address: 0x%08x\n", breakpoint_address);
					breakpoint->stepOver(target_pid);
				}

				// Continue execution of child process
				continue;
			}
			else
			{
				procmsg("[DEBUG] Child process stopped - unknown signal! (%d)\n", last_sig);

				// TODO: Check for other signal types
				// TODO: Continue debugging instead of exiting debug loop
				break;
			}
		}
	}

	return true;
}