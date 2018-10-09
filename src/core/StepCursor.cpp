#include "StepCursor.hpp"

#include <set>

#include "Unwinder.hpp"

StepCursor::StepCursor(std::shared_ptr<DebugInfo> debug_info,
                       std::shared_ptr<BreakpointTable> user_breakpoints)
{
	this->debug_info = debug_info;
	this->user_breakpoints = user_breakpoints;
}

void StepCursor::stepOver(ProcessTracer& tracer)
{
	// Step over a user breakpoint if one is present at the current address
	if (isStoppedAtBreakpoint(*user_breakpoints, tracer))
		stepOverBreakpoint(*user_breakpoints, tracer);

	// Single step over the current instruction to avoid getting stuck on a
	// breakpoint on the same line
	uint64_t pre_step_address = getCurrentAddress(tracer);
	uint64_t pre_step_ret_address = getReturnAddress(tracer.traceePID());
	tracer.singleStepExec();

	// Create breakpoints on every line of this function apart from the current,
	// and enable them
	BreakpointTable internal_breakpoints(debug_info);
	addSubprogramBreakpoints(internal_breakpoints, tracer, pre_step_address);
	addReturnBreakpoint(internal_breakpoints, tracer, pre_step_ret_address);

	// Continue until the next breakpoint is hit
	tracer.continueExec();

	// Disable all internal breakpoints
	internal_breakpoints.disableBreakpoints(tracer);

	// If it's an internal breakpoint, rewind the IP by 1 so that the
	// instruction that was hidden by the breakpoint will be the next to be
	// executed.
	if (isStoppedAtBreakpoint(internal_breakpoints, tracer))
		rewindIP(tracer);
}

void StepCursor::stepInto(ProcessTracer& tracer)
{
	// Step over a user breakpoint if one is present at the current address
	if (isStoppedAtBreakpoint(*user_breakpoints, tracer))
		stepOverBreakpoint(*user_breakpoints, tracer);

	// Get the current and return addresses, then single step to avoid a
	// breakpoint set on the current instruction
	uint64_t pre_step_address = getCurrentAddress(tracer);
	uint64_t pre_step_ret_address = getReturnAddress(tracer.traceePID());
	bool has_made_call = isCallInstruction(pre_step_address, tracer);
	tracer.singleStepExec();

	// Initialise and enable breakpoints for all lines in the current function
	// as well as a breakpoint on the line after the return address
	BreakpointTable internal_breakpoints(debug_info);
	addSubprogramBreakpoints(internal_breakpoints, tracer, pre_step_address);
	addReturnBreakpoint(internal_breakpoints, tracer, pre_step_ret_address);

	// Continue single-stepping until a call instruction is executed or a
	// breakpoint is encountered
	while (!hasHitBreakpoint(internal_breakpoints, tracer) && !has_made_call)
	{
		uint64_t current_address = getCurrentAddress(tracer);
		has_made_call = isCallInstruction(current_address, tracer);

		tracer.singleStepExec();
	}

	// Disable the internal breakpoints
	internal_breakpoints.disableBreakpoints(tracer);

	// If it's an internal breakpoint, rewind the IP by 1 so that the
	// instruction that was hidden by the breakpoint will be the next to be
	// executed.
	if (isStoppedAtBreakpoint(internal_breakpoints, tracer))
		rewindIP(tracer);
}

void StepCursor::stepOut(ProcessTracer& tracer)
{
	// Step over a user breakpoint if one is present at the current address
	if (isStoppedAtBreakpoint(*user_breakpoints, tracer))
		stepOverBreakpoint(*user_breakpoints, tracer);

	// Get the return address, then single step to avoid a breakpoint set on the
	// current instruction
	uint64_t pre_step_ret_address = getReturnAddress(tracer.traceePID());
	tracer.singleStepExec();

	// Initialise and enable a breakpoint for the next line after the return
	// address
	BreakpointTable internal_breakpoints(debug_info);
	addReturnBreakpoint(internal_breakpoints, tracer, pre_step_ret_address);

	// Continue execution util a breakpoint is hit
	tracer.continueExec();

	// Disable the internal breakpoints
	internal_breakpoints.disableBreakpoints(tracer);

	// If it's an internal breakpoint, rewind the IP by 1 so that the
	// instruction that was hidden by the breakpoint will be the next to be
	// executed.
	if (isStoppedAtBreakpoint(internal_breakpoints, tracer))
		rewindIP(tracer);
}

uint64_t StepCursor::getCurrentAddress(ProcessTracer& tracer)
{
	auto expected_regs = tracer.getRegisters();
	assert(expected_regs.has_value());
	user_regs_struct regs = expected_regs.value();
	return regs.rip;
}

uint64_t StepCursor::getCurrentLineNumber(ProcessTracer& tracer)
{
	uint64_t current_address = getCurrentAddress(tracer);
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

std::string StepCursor::getCurrentSourceFile(ProcessTracer& tracer)
{
	uint64_t current_address = getCurrentAddress(tracer);
	auto expected_function = debug_info->getFunction(current_address);
	assert(expected_function.has_value() &&
	       "Current instruction address does not belong to a known function!");
	return expected_function.value().decl_file;
}

void StepCursor::addSubprogramBreakpoints(BreakpointTable &internal,
                                          ProcessTracer& tracer,
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
			internal.getBreakpoint(line.address).enable(tracer);
			used_lines.insert(line.number);
		}
	}
}

void StepCursor::addReturnBreakpoint(BreakpointTable &internal,
                                     ProcessTracer& tracer,
                                     uint64_t address)
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
		internal.getBreakpoint(next_closest_address).enable(tracer);
	}
}

uint64_t StepCursor::getReturnAddress(pid_t pid)
{
	Unwinder unwinder(pid);
	unwinder.unwindStep();
	return unwinder.getRegisterValue(UNW_REG_IP);
}

bool StepCursor::isStoppedAtBreakpoint(BreakpointTable& table, ProcessTracer& tracer)
{
	uint64_t breakpoint_address = getCurrentAddress(tracer) - 1;
	return table.isBreakpoint(breakpoint_address);
}

void StepCursor::stepOverBreakpoint(BreakpointTable& table, ProcessTracer& tracer)
{
	uint64_t breakpoint_address = getCurrentAddress(tracer) - 1;
	Breakpoint& breakpoint = table.getBreakpoint(breakpoint_address);
	breakpoint.stepOver(tracer);
}

bool StepCursor::isCallInstruction(uint64_t address, ProcessTracer& tracer)
{
	auto expected_data = tracer.peekText(address);
	assert(expected_data.has_value());
	uint64_t data = expected_data.value();
	return (data & 0xE8) == 0xE8;
}

bool StepCursor::hasHitBreakpoint(BreakpointTable &internal, ProcessTracer& tracer)
{
	uint64_t bp_address = getCurrentAddress(tracer) - 1;
	bool hit_user_bp = user_breakpoints->isBreakpoint(bp_address);
	bool hit_internal_bp = internal.isBreakpoint(bp_address);
	return hit_user_bp || hit_internal_bp;
}

void StepCursor::rewindIP(ProcessTracer& tracer)
{
	auto expected_regs = tracer.getRegisters();
	assert(expected_regs.has_value());
	user_regs_struct regs = expected_regs.value();
	regs.rip = getCurrentAddress(tracer) - 1;
	tracer.setRegisters(regs);
}