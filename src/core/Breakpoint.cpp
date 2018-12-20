#include "Breakpoint.hpp"

// DEBUG: Move these functions to their own dedicated file.
void procmsg(const char *format, ...);
unsigned getChildInstructionPointer(pid_t child_pid);

Breakpoint::Breakpoint(uint64_t addr)
{
	this->addr = addr;

	procmsg("Breakpoint created: 0x%08x\n", addr);
}

Breakpoint::Breakpoint(Breakpoint &&other)
{
	this->addr = other.addr;
	this->orig_data = other.orig_data;
}

// Enables this breakpoint by replacing the instruction at its assigned address
// with an 'int 3' trap instruction.
// The original instruction at the address is saved and can be restored by
// disabling this breakpoint.
void Breakpoint::enable(ProcessTracer& tracer)
{
	auto expected_orig_data = tracer.peekText(addr);
	assert(expected_orig_data.has_value());
	orig_data = expected_orig_data.value();
	tracer.pokeText(addr, (orig_data & ~(0xFF)) | 0xCC);

	procmsg("[DEBUG] Pre-enabled data at 0x%08x: 0x%08x\n", addr, orig_data);
	procmsg("[DEBUG] Post-enabled data at 0x%08x: 0x%08x\n", addr, tracer.peekText(addr).value());
}

// Disables this breakpoint by replacing the 'int 3' trap instruction at its
// assigned address with the original instruction.
// The original instruction at the address is saved and can be restored by
// enabling this breakpoint.
void Breakpoint::disable(ProcessTracer& tracer)
{
	auto expected_data = tracer.peekText(addr);
	assert(expected_data.has_value());
	uint64_t data = expected_data.value();

    // Ensure that the instruction being replaced is a trap 'int 3' breakpoint instruction
	assert((data & 0xFF) == 0xCC);
	tracer.pokeText(addr, (data & ~(0xFF)) | (orig_data & 0xFF));

	procmsg("[DEBUG] Pre-disabled data at 0x%08x: 0x%08x\n", addr, data);
	procmsg("[DEBUG] Post-disabled data at 0x%08x: 0x%08x\n", addr, tracer.peekText(addr).value());
}

// Disables this breakpoint, steps over it and then re-enables the breakpoint.
bool Breakpoint::stepOver(ProcessTracer& tracer)
{
	// Get registers
	auto expected_regs = tracer.getRegisters();
	assert(expected_regs.has_value());
	user_regs_struct regs = expected_regs.value();

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
	tracer.setRegisters(regs);

	// Temporarily disable this breakpoint
	disable(tracer);

	// Single-step over the instruction
	tracer.singleStepExec();

	// Re-enable this breakpoint
	enable(tracer);

	// Check if the child has exited after stepping
	if (!tracer.isRunning())
	{
		return false;
	}

	return true;
}

Breakpoint &Breakpoint::operator=(Breakpoint &&other)
{
	this->addr = other.addr;
	this->orig_data = other.orig_data;
}