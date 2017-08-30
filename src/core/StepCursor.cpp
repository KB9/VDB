#include "StepCursor.hpp"

#include "Unwinder.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

StepCursor::StepCursor(uint64_t address, std::shared_ptr<DwarfDebug> debug_data,
                       std::shared_ptr<BreakpointTable> breakpoint_table)
{
	this->debug_data = debug_data;
	this->breakpoint_table = breakpoint_table;
	updateTrackingVars(address);
}

void StepCursor::stepOver(pid_t pid)
{
	// Step over a user breakpoint if current execution was halted by one
	user_regs_struct regs;
	ptrace(PTRACE_GETREGS, pid, 0, &regs);
	std::unique_ptr<Breakpoint> start_breakpoint = breakpoint_table->getBreakpoint(regs.rip - 1);
	if (start_breakpoint != nullptr)
	{
		start_breakpoint->stepOver(pid);
	}

	// Perform the step to the next source line, update the tracking variables
	uint64_t next_address = stepToNextSourceLine(pid, address);
	updateTrackingVars(next_address);
}

void StepCursor::stepInto(pid_t pid)
{
	// Step over a user breakpoint if current execution was halted by one
	DIESubprogram *current = getSubprogramFromAddress(address);

	user_regs_struct regs;
	ptrace(PTRACE_GETREGS, pid, 0, &regs);
	std::unique_ptr<Breakpoint> start_breakpoint = breakpoint_table->getBreakpoint(regs.rip - 1);
	if (start_breakpoint != nullptr)
	{
		start_breakpoint->stepOver(pid);
	}

	// Create the internal breakpoints for the current function, avoiding the
	// current line to prevent infinitely landing on the same breakpoint
	std::unique_ptr<BreakpointTable> internal_breakpoints =
			createSubprogramBreakpoints(pid, address, false);

	// Enable the internal breakpoints for the current function
	internal_breakpoints->enableBreakpoints(pid);

	// Continue single-stepping until a new function has been entered, or a
	// breakpoint in the current function has been hit (ignoring the breakpoint
	// that was started on)
	while (getSubprogramFromAddress(regs.rip)->name == current->name &&
	       internal_breakpoints->getBreakpoint(regs.rip - 1) == nullptr &&
	       (breakpoint_table->getBreakpoint(regs.rip - 1) == nullptr ||
	       breakpoint_table->getBreakpoint(regs.rip - 1)->addr == start_breakpoint->addr))
	{
		int wait_status;
		if (ptrace(PTRACE_SINGLESTEP, pid, 0, 0))
		{
			perror("ptrace");
			return;
		}
		wait(&wait_status);

		ptrace(PTRACE_GETREGS, pid, 0, &regs);
	}

	// Disable the internal breakpoints again
	internal_breakpoints->disableBreakpoints(pid);

	// If the step cursor hit an internal breakpoint i.e. execution did not
	// leave the current function
	if (internal_breakpoints->getBreakpoint(regs.rip - 1) != nullptr)
	{
		// If it's an internal breakpoint, rewind the IP by 1 so that it the
		// instruction that was hidden by the breakpoint will be the next to be
		// executed.
		ptrace(PTRACE_GETREGS, pid, 0, &regs);
		regs.rip = regs.rip - 1;
		ptrace(PTRACE_SETREGS, pid, 0, &regs);

		updateTrackingVars(regs.rip);
	}
	// If the step cursor hit a user breakpoint i.e. execution did not leave
	// the current function
	else if (breakpoint_table->getBreakpoint(regs.rip - 1) != nullptr)
	{
		// If it's a user breakpoint, pretend that the IP has been rewound by 1
		// so that this breakpoint can be stepped over using the real IP if needed
		// (this return value is only used to update the tracking address).
		updateTrackingVars(regs.rip - 1);
	}
	// If the subprogram name changed i.e. execution entered a new function
	else if (getSubprogramFromAddress(regs.rip)->name != current->name)
	{
		// Continue to the next source line
		//uint64_t next_address = stepToNextSourceLine(pid, regs.rip);
		//updateTrackingVars(next_address);

		// Update the tracking variables with the 1st address in the new function
		updateTrackingVars(regs.rip);
	}
	// Execution should not get here
	else
	{
		assert(false);
		return;
	}
}

void StepCursor::stepOut(pid_t pid)
{
	// Step over a user breakpoint if current execution was halted by one
	user_regs_struct regs;
	ptrace(PTRACE_GETREGS, pid, 0, &regs);
	std::unique_ptr<Breakpoint> start_breakpoint = breakpoint_table->getBreakpoint(regs.rip - 1);
	if (start_breakpoint != nullptr)
	{
		start_breakpoint->stepOver(pid);
	}

	// Perform the step to the calling function, update the tracking variables
	uint64_t next_address = stepToCallingFunction(pid, address);
	updateTrackingVars(next_address);
}

uint64_t StepCursor::getCurrentAddress()
{
	return address;
}

uint64_t StepCursor::getCurrentLineNumber()
{
	return line_number;
}

std::string StepCursor::getCurrentSourceFile()
{
	return source_file;
}

uint64_t StepCursor::stepToNextSourceLine(pid_t pid, uint64_t addr,
                                          bool include_current_addr)
{
	// Create the interal step breakpoints for the current subprogram
	std::unique_ptr<BreakpointTable> internal_breakpoints =
			createSubprogramBreakpoints(pid, addr, include_current_addr);

	// Enable all internal breakpoints so that they can halt execution
	internal_breakpoints->enableBreakpoints(pid);

	// Continue until the next breakpoint is hit
	int wait_status;
	if (ptrace(PTRACE_CONT, pid, 0, 0) < 0)
	{
		perror("ptrace");
		return 0;
	}
	wait(&wait_status);

	// Disable all internal breakpoints and decide which address to return
	internal_breakpoints->disableBreakpoints(pid);
	user_regs_struct regs;
	ptrace(PTRACE_GETREGS, pid, 0, &regs);
	if (internal_breakpoints->getBreakpoint(regs.rip - 1) != nullptr)
	{
		// If it's an internal breakpoint, rewind the IP by 1 so that it the
		// instruction that was hidden by the breakpoint will be the next to be
		// executed.
		ptrace(PTRACE_GETREGS, pid, 0, &regs);
		regs.rip = regs.rip - 1;
		ptrace(PTRACE_SETREGS, pid, 0, &regs);

		return regs.rip;
	}
	else if (breakpoint_table->getBreakpoint(regs.rip - 1) != nullptr)
	{
		// If it's a user breakpoint, pretend that the IP has been rewound by 1
		// so that this breakpoint can be stepped over using the real IP if needed
		// (this return value is only used to update the tracking address).
		return regs.rip - 1;
	}
	else
	{
		// Execution should not reach this point. If execution reaches here, it
		// means there was a serious problem with the debug target process.
		assert(false && "Step cursor did not hit the next breakpoint!");
		return 0;
	}
}

uint64_t StepCursor::stepToCallingFunction(pid_t pid, uint64_t addr)
{
	// Create the interal step breakpoint to return to the calling function
	std::unique_ptr<BreakpointTable> internal_breakpoints = createReturnBreakpoint(pid, addr);

	// Enable the internal breakpoint so that it can halt execution
	internal_breakpoints->enableBreakpoints(pid);

	// Continue until the next breakpoint is hit
	int wait_status;
	if (ptrace(PTRACE_CONT, pid, 0, 0) < 0)
	{
		perror("ptrace");
		return 0;
	}
	wait(&wait_status);

	// Disable the internal breakpoint and decide which address to return
	internal_breakpoints->disableBreakpoints(pid);
	user_regs_struct regs;
	ptrace(PTRACE_GETREGS, pid, 0, &regs);
	if (internal_breakpoints->getBreakpoint(regs.rip - 1) != nullptr)
	{
		// If it's an internal breakpoint, rewind the IP by 1 so that it the
		// instruction that was hidden by the breakpoint will be the next to be
		// executed.
		ptrace(PTRACE_GETREGS, pid, 0, &regs);
		regs.rip = regs.rip - 1;
		ptrace(PTRACE_SETREGS, pid, 0, &regs);

		return regs.rip;
	}
	else if (breakpoint_table->getBreakpoint(regs.rip - 1) != nullptr)
	{
		// If it's a user breakpoint, pretend that the IP has been rewound by 1
		// so that this breakpoint can be stepped over using the real IP if needed
		// (this return value is only used to update the tracking address).
		return regs.rip - 1;
	}
	else
	{
		// Execution should not reach this point. If execution reaches here, it
		// means there was a serious problem with the debug target process.
		assert(false && "Step cursor did not hit the next breakpoint!");
		return 0;
	}
}

DIESubprogram *StepCursor::getSubprogramFromAddress(uint64_t address)
{
	std::vector<CUHeader> cu_headers = debug_data->info()->getCUHeaders();
	DIESubprogram *current_prog = nullptr;
	for (auto header : cu_headers)
	{
		SharedPtrVector<DIESubprogram> sub_progs = header.getDIEsOfType<DIESubprogram>();
		for (auto prog : sub_progs)
		{
			// TODO: Be careful of boundary calculation between DWARF versions;
			// will need to ensure this for DIESubprograms.
			if (address >= prog->lowpc && address < (prog->lowpc + prog->highpc))
			{
				current_prog = prog.get();
				break;
			}
		}
	}
	if (current_prog != nullptr) procmsg("%s\n", current_prog->name.c_str());
	return current_prog;
}

std::unique_ptr<BreakpointTable> StepCursor::createSubprogramBreakpoints(pid_t pid,
                                                                         uint64_t addr,
                                                                         bool include_current_addr)
{
	// Initialize the internal breakpoint table
	auto internal_breakpoints = std::make_unique<BreakpointTable>(debug_data);

	// Set breakpoints on lines which don't have a user breakpoint, and which
	// aren't the line currently stopped on
	std::vector<Line> lines = debug_data->line()->getAllLines();
	DIESubprogram *current = getSubprogramFromAddress(addr);
	for (auto line : lines)
	{
		if (line.address >= current->lowpc &&
		    line.address < (current->lowpc + current->highpc) &&
		    breakpoint_table->getBreakpoint(line.address) == nullptr)
		{
			// If the line address is not equal to the current address, add it
			if (line.address != addr)
			{
				internal_breakpoints->addBreakpoint(line.address);
			}
			// If the line address is equal to the current address and the
			// current address inclusion flag has been enabled, add it
			else if (include_current_addr)
			{
				internal_breakpoints->addBreakpoint(line.address);
			}
		}
	}

	// Include a breakpoint at the point in the code that this function returns
	// to as well (if there isn't already a user breakpoint there)
	Unwinder unwinder(pid);
	unwinder.unwindStep();
	uint64_t return_address = unwinder.getRegisterValue(UNW_REG_IP);
	if (breakpoint_table->getBreakpoint(return_address) == nullptr)
		internal_breakpoints->addBreakpoint(return_address);

	return std::move(internal_breakpoints);
}

std::unique_ptr<BreakpointTable> StepCursor::createReturnBreakpoint(pid_t pid,
                                                                    uint64_t addr)
{
	// Initialize the internal breakpoint table
	auto internal_breakpoints = std::make_unique<BreakpointTable>(debug_data);

	// Include a breakpoint at the point in the code that this function returns
	// to (if there isn't already a user breakpoint there)
	Unwinder unwinder(pid);
	unwinder.unwindStep();
	uint64_t return_address = unwinder.getRegisterValue(UNW_REG_IP);
	if (breakpoint_table->getBreakpoint(return_address) == nullptr)
		internal_breakpoints->addBreakpoint(return_address);

	return std::move(internal_breakpoints);
}

void StepCursor::updateTrackingVars(uint64_t addr)
{
	this->address = addr;

	DebuggingInformationEntry *die = getSubprogramFromAddress(addr);
	while (die != nullptr && dynamic_cast<DIECompileUnit *>(die) == nullptr)
	{
		die = die->getParent();
	}
	// There should always be a compile unit DIE at the DIE tree root
	assert(die != nullptr);
	// Determine the path to the source file which contains the subprogram
	DIECompileUnit *cu = dynamic_cast<DIECompileUnit *>(die);
	std::string src_file = cu->getCompDir() + "/" + cu->getName();

	std::vector<Line> lines = debug_data->line()->getAllLines();
	uint64_t src_line_number = 0;
	for (auto line : lines)
	{
		if (line.address == addr)
		{
			src_line_number = line.number;
			break;
		}
	}

	this->source_file = src_file;
	this->line_number = src_line_number;

	procmsg("[STEP_CURSOR] Tracking vars updated: address = 0x%x, source = %s, line = %llu\n",
	        address, source_file.c_str(), line_number);
}