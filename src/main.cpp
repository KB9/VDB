#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <unistd.h>
#include <errno.h>

#include <cstring>

#include "Breakpoint.hpp"
#include "DwarfCpp.hpp"

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

void run_target(const char *program_name)
{
	procmsg("Target started. Will run '%s'\n", program_name);

	// Allow tracing of this process
	if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0)
	{
		perror("ptrace");
		return;
	}

	// Replace this process's image with the given program
	execl(program_name, program_name, 0);
}

void trace(pid_t child_pid)
{
	procmsg("Tracing entire program");

	// Wait for child to stop on its first instruction
	procmsg("Child now at EIP = 0x%08x\n", getChildInstructionPointer(child_pid));

	int instr_count = 0;
	int wait_status;
	wait(&wait_status);
	while (WIFSTOPPED(wait_status))
	{
		instr_count++;
		
		unsigned instr = ptrace(PTRACE_PEEKTEXT, child_pid, getChildInstructionPointer(child_pid), 0);
		procmsg("EIP = 0x%08x, instr = 0x%08x\n", getChildInstructionPointer(child_pid), instr);

		if (ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0) < 0)
		{
			perror("ptrace");
			return;
		}

		wait(&wait_status);
	}

	procmsg("Instructions executed: %d\n", instr_count);
}

void run_debugger(pid_t child_pid)
{
	// DWARF parser testing
	DebugInfo debug_info;

	int wait_status;

	// Wait for child to stop on its first instruction
	wait(0);
	procmsg("Entry point. EIP = 0x%08x\n", getChildInstructionPointer(child_pid));

	// DEBUG: Set a breakpoint after child has reached its first instruction
	Breakpoint breakpoint(child_pid, (void *)0x400566); // For ../../../testchild
	
	while (true)
	{	
		// Resume execution
		if (ptrace(PTRACE_CONT, child_pid, 0, 0) < 0)
		{
			perror("ptrace");
			return; // TODO: Should really return an error code
		}
		wait(&wait_status);

		// If the child process exited
		if (WIFEXITED(wait_status))
		{
			// Stop debugging
			break;
		}
		// If the child process stopped
		else if (WIFSTOPPED(wait_status))
		{
			int last_sig = WSTOPSIG(wait_status);

			// If a breakpoint was hit
			if (last_sig == SIGTRAP)
			{
				procmsg("[DEBUG] Breakpoint hit!\n");
				
				// TODO: Find the correct breakpoint
				// TODO: Do stuff at this breakpoint

				// DEBUG: There's only 1 breakpoint currently, step over it
				if (!breakpoint.stepOver(child_pid)) break;
				
                // Continue execution of child process
				continue;
			}
			else
			{
				procmsg("[DEBUG] Child process stopped - unknown signal! (%d)\n", last_sig);
				
				// TODO: Check for other last signal types
				break; // TODO: I should really continue - but that causes it to loop forever
			}
		}
	}
}

int main(int argc, char **argv)
{	
	pid_t child_pid;

	if (argc < 2)
	{
		fprintf(stderr, "Expected a program name as argument\n");
		return -1;
	}

	child_pid = fork();
	if (child_pid == 0)
	{
		run_target(argv[1]);
	}
	else if (child_pid > 0)
	{
		run_debugger(child_pid);
	}
	else
	{
		perror("fork");
		return -1;
	}

	return 0;
}
