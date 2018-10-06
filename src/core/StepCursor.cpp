#include "StepCursor.hpp"

#include <set>

#include "Unwinder.hpp"

StepCursor::StepCursor(std::shared_ptr<DebugInfo> debug_info,
                       std::shared_ptr<BreakpointTable> user_breakpoints)
{
	this->debug_info = debug_info;
	this->user_breakpoints = user_breakpoints;
}

void StepCursor::stepOver(pid_t pid)
{
	// Step over a user breakpoint if one is present at the current address
	if (isStoppedAtUserBreakpoint(pid))
		stepOverUserBreakpoint(pid);

	// Single step over the current instruction to avoid getting stuck on a
	// breakpoint on the same line
	uint64_t pre_step_address = getCurrentAddress(pid);
	uint64_t pre_step_ret_address = getReturnAddress(pid);
	singleStep(pid);

	// Create breakpoints on every line of this function apart from the current,
	// and enable them
	BreakpointTable internal_breakpoints(debug_info);
	addSubprogramBreakpoints(pid, internal_breakpoints, pre_step_address);
	addReturnBreakpoint(pid, internal_breakpoints, pre_step_ret_address);

	// Continue until the next breakpoint is hit
	continueExec(pid);

	// Disable all internal breakpoints
	internal_breakpoints.disableBreakpoints(pid);

	// If it's an internal breakpoint, rewind the IP by 1 so that the
	// instruction that was hidden by the breakpoint will be the next to be
	// executed.
	uint64_t breakpoint_address = getCurrentAddress(pid) - 1;
	if (internal_breakpoints.isBreakpoint(breakpoint_address))
	{
		user_regs_struct regs;
		ptrace(PTRACE_GETREGS, pid, 0, &regs);
		regs.rip = breakpoint_address;
		ptrace(PTRACE_SETREGS, pid, 0, &regs);
	}
}

void StepCursor::stepInto(pid_t pid)
{
	// Step over a user breakpoint if one is present at the current address
	if (isStoppedAtUserBreakpoint(pid))
		stepOverUserBreakpoint(pid);

	// Get the current and return addresses, then single step to avoid a
	// breakpoint set on the current instruction
	uint64_t pre_step_address = getCurrentAddress(pid);
	uint64_t pre_step_ret_address = getReturnAddress(pid);
	bool has_made_call = isCallInstruction(pid, pre_step_address);
	singleStep(pid);

	// Initialise and enable breakpoints for all lines in the current function
	// as well as a breakpoint on the line after the return address
	BreakpointTable internal_breakpoints(debug_info);
	addSubprogramBreakpoints(pid, internal_breakpoints, pre_step_address);
	addReturnBreakpoint(pid, internal_breakpoints, pre_step_ret_address);

	// Continue single-stepping until a call instruction is executed or a
	// breakpoint is encountered
	while (!hasHitBreakpoint(pid, internal_breakpoints) && !has_made_call)
	{
		uint64_t current_address = getCurrentAddress(pid);
		has_made_call = isCallInstruction(pid, current_address);

		singleStep(pid);
	}

	// Disable the internal breakpoints
	internal_breakpoints.disableBreakpoints(pid);

	// If it's an internal breakpoint, rewind the IP by 1 so that the
	// instruction that was hidden by the breakpoint will be the next to be
	// executed.
	uint64_t breakpoint_address = getCurrentAddress(pid) - 1;
	if (internal_breakpoints.isBreakpoint(breakpoint_address))
	{
		user_regs_struct regs;
		ptrace(PTRACE_GETREGS, pid, 0, &regs);
		regs.rip = breakpoint_address;
		ptrace(PTRACE_SETREGS, pid, 0, &regs);
	}
}

void StepCursor::stepOut(pid_t pid)
{
	// Step over a user breakpoint if one is present at the current address
	if (isStoppedAtUserBreakpoint(pid))
		stepOverUserBreakpoint(pid);

	// Get the return address, then single step to avoid a breakpoint set on the
	// current instruction
	uint64_t pre_step_ret_address = getReturnAddress(pid);
	singleStep(pid);

	// Initialise and enable a breakpoint for the next line after the return
	// address
	BreakpointTable internal_breakpoints(debug_info);
	addReturnBreakpoint(pid, internal_breakpoints, pre_step_ret_address);

	// Continue execution util a breakpoint is hit
	continueExec(pid);

	// Disable the internal breakpoints
	internal_breakpoints.disableBreakpoints(pid);

	// If it's an internal breakpoint, rewind the IP by 1 so that the
	// instruction that was hidden by the breakpoint will be the next to be
	// executed.
	uint64_t breakpoint_address = getCurrentAddress(pid) - 1;
	if (internal_breakpoints.isBreakpoint(breakpoint_address))
	{
		user_regs_struct regs;
		ptrace(PTRACE_GETREGS, pid, 0, &regs);
		regs.rip = breakpoint_address;
		ptrace(PTRACE_SETREGS, pid, 0, &regs);
	}
}

uint64_t StepCursor::getCurrentAddress(pid_t pid)
{
	user_regs_struct regs;
	ptrace(PTRACE_GETREGS, pid, 0, &regs);
	return regs.rip;
}

uint64_t StepCursor::getCurrentLineNumber(pid_t pid)
{
	uint64_t current_address = getCurrentAddress(pid);
	auto lines = debug_info->getFunctionLines(current_address);
	for (auto line : lines)
	{
		if (line.address == current_address)
		{
			return line.number;
		}
	}
	assert(!"Current instruction address does not belong to a known function!");
}

std::string StepCursor::getCurrentSourceFile(pid_t pid)
{
	uint64_t current_address = getCurrentAddress(pid);
	auto expected_function = debug_info->getFunction(current_address);
	assert(expected_function.has_value() &&
	       "Current instruction address does not belong to a known function!");
	return expected_function.value().decl_file;
}

void StepCursor::addSubprogramBreakpoints(pid_t pid, BreakpointTable &internal,
                                          uint64_t address)
{
	// Set breakpoints on lines which don't have a user breakpoint, and which
	// aren't the line currently stopped on (if desired)
	auto lines = debug_info->getFunctionLines(address);
	std::set<uint64_t> used_lines;
	for (const auto &line : lines)
	{
		if (!user_breakpoints->isBreakpoint(line.address) &&
		    !internal.isBreakpoint(line.address) &&
		    used_lines.find(line.number) == used_lines.end())
		{
			internal.addBreakpoint(line.address);
			internal.getBreakpoint(line.address).enable(pid);
			used_lines.insert(line.number);
		}
	}
}

void StepCursor::addReturnBreakpoint(pid_t pid, BreakpointTable &internal, uint64_t address)
{
	auto lines = debug_info->getFunctionLines(address);
	uint64_t next_closest_address = std::numeric_limits<uint64_t>::max();
	bool address_found = false;
	for (const auto &line : lines)
	{
		bool is_higher_address = line.address >= address;
		bool is_closer_address = line.address < next_closest_address;
		if (is_higher_address && is_closer_address)
		{
			next_closest_address = line.address;
			address_found = true;
		}
	}

	if (!user_breakpoints->isBreakpoint(next_closest_address) &&
	    !internal.isBreakpoint(next_closest_address) &&
	    address_found)
	{
		internal.addBreakpoint(next_closest_address);
		internal.getBreakpoint(next_closest_address).enable(pid);
	}
}

uint64_t StepCursor::getReturnAddress(pid_t pid)
{
	Unwinder unwinder(pid);
	unwinder.unwindStep();
	return unwinder.getRegisterValue(UNW_REG_IP);
}

bool StepCursor::isStoppedAtUserBreakpoint(pid_t pid)
{
	uint64_t breakpoint_address = getCurrentAddress(pid) - 1;
	return user_breakpoints->isBreakpoint(breakpoint_address);
}

void StepCursor::stepOverUserBreakpoint(pid_t pid)
{
	uint64_t breakpoint_address = getCurrentAddress(pid) - 1;
	Breakpoint &breakpoint = user_breakpoints->getBreakpoint(breakpoint_address);
	breakpoint.stepOver(pid);
}

bool StepCursor::isCallInstruction(pid_t pid, uint64_t address)
{
	uint64_t data = ptrace(PTRACE_PEEKTEXT, pid, address, 0);
	return (data & 0xE8) == 0xE8;
}

bool StepCursor::hasHitBreakpoint(pid_t pid, BreakpointTable &internal)
{
	uint64_t bp_address = getCurrentAddress(pid) - 1;
	bool hit_user_bp = user_breakpoints->isBreakpoint(bp_address);
	bool hit_internal_bp = internal.isBreakpoint(bp_address);
	return hit_user_bp || hit_internal_bp;
}

bool StepCursor::singleStep(pid_t pid)
{
	int wait_status;
	if (ptrace(PTRACE_SINGLESTEP, pid, 0, 0))
	{
		perror("ptrace");
		return false;
	}
	wait(&wait_status);
	return true;
}

bool StepCursor::continueExec(pid_t pid)
{
	int wait_status;
	if (ptrace(PTRACE_CONT, pid, 0, 0) < 0)
	{
		perror("ptrace");
		return false;
	}
	wait(&wait_status);
	return true;
}