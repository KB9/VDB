#include "Breakpoint.hpp"

// DEBUG: Move these functions to their own dedicated file.
void procmsg(const char *format, ...);
unsigned getChildInstructionPointer(pid_t child_pid);

Breakpoint::Breakpoint(pid_t pid, void *addr)
{
	this->addr = addr;
	enable(pid);

	procmsg("Breakpoint created: 0x%08x\n", addr);
}

// Enables this breakpoint by replacing the instruction at its assigned address
// with an 'int 3' trap instruction.
// The original instruction at the address is saved and can be restored by
// disabling this breakpoint.
void Breakpoint::enable(pid_t pid)
{
	orig_data = ptrace(PTRACE_PEEKTEXT, pid, addr, 0);
	ptrace(PTRACE_POKETEXT, pid, addr, (orig_data & ~(0xFF)) | 0xCC);

	procmsg("[DEBUG] Pre-enabled data at 0x%08x: 0x%08x\n", addr, orig_data);
	procmsg("[DEBUG] Post-enabled data at 0x%08x: 0x%08x\n", addr, ptrace(PTRACE_PEEKTEXT, pid, addr, 0));
}

// Disables this breakpoint by replacing the 'int 3' trap instruction at its
// assigned address with the original instruction.
// The original instruction at the address is saved and can be restored by
// enabling this breakpoint.
void Breakpoint::disable(pid_t pid)
{
	unsigned data = ptrace(PTRACE_PEEKTEXT, pid, addr, 0);
    // Ensure that the instruction being replaced is a trap 'int 3' breakpoint instruction
	assert((data & 0xFF) == 0xCC);
	ptrace(PTRACE_POKETEXT, pid, addr, (data & ~(0xFF)) | (orig_data & 0xFF));

	procmsg("[DEBUG] Pre-disabled data at 0x%08x: 0x%08x\n", addr, data);
	procmsg("[DEBUG] Post-disabled data at 0x%08x: 0x%08x\n", addr, ptrace(PTRACE_PEEKTEXT, pid, addr, 0));
}

// TODO: Think about this function.
// The single-step and re-enabling is required before execution of the child
// process can continue. However, should this function be the one responsible
// for resuming execution? The callee should probably continue and listen for
// SIGTRAP, and then look at all enabled breakpoints to determine which one
// it should resume from.
bool Breakpoint::stepOver(pid_t pid)
{
	user_regs_struct regs;
	int wait_status;

	// Get registers
	ptrace(PTRACE_GETREGS, pid, 0, &regs);

	procmsg("[DEBUG] Resuming from EIP = 0x%08x\n", regs.rip);

	// Make sure we are stopped at this breakpoint
#if defined ENV64
	assert(regs.rip == (unsigned long) addr + 1);
#elif defined ENV32
	assert(regs.eip == (long) addr + 1);
#endif

	// Set the instruction pointer to this breakpoint's address
#if defined ENV64
	regs.rip = (long) addr;
#elif defined ENV32
	regs.eip = (long) addr;
#endif
	ptrace(PTRACE_SETREGS, pid, 0, &regs);

	// Temporarily disable this breakpoint
	disable(pid);

	procmsg("[DEBUG] Pre-step EIP = 0x%08x\n", getChildInstructionPointer(pid));
	if (ptrace(PTRACE_SINGLESTEP, pid, 0, 0))
	{
		perror("ptrace");
		return false;
	}
	wait(&wait_status);
	procmsg("[DEBUG] Post-step EIP = 0x%08x\n", getChildInstructionPointer(pid));

	// Check if the child has exited after stepping
	if (WIFEXITED(wait_status))
	{
		return false;
	}

	return true;
}
