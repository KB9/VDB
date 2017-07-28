#include "StepCursor.hpp"

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
	// Get the current subprogram that is being executed
	DIESubprogram *current = getSubprogramFromAddress(address);

	user_regs_struct regs;
	ptrace(PTRACE_GETREGS, pid, 0, &regs);

	// If a step-over is being carried out from the last line of the current
	// subprogram
	Line subprogram_last_line = getLastLineOfSubprogram(*current);
	if (regs.rip == subprogram_last_line.address)
	{
		// Simply jump to the next line as the end of the current
		// subprogram has been reached
		uint64_t next_address = stepToNextSourceLine(pid, regs.rip);
		updateTrackingVars(next_address);
	}
	else
	{
		// It's not the last line of the current subprogram, therefore execution
		// must return here at some point.
		// Step to the address of the next source line to be executed in the
		// same subprogram.
		uint64_t next_address = stepToNextSourceLine(pid, address);
		while (getSubprogramFromAddress(next_address)->name != current->name)
		{
			next_address = stepToNextSourceLine(pid, next_address);
		}

		// Update the tracking vars
		updateTrackingVars(next_address);
	}
}

void StepCursor::stepInto(pid_t pid)
{
	// Step to the address of the next source line to be executed
	uint64_t next_address = stepToNextSourceLine(pid, address);

	// Update the tracking vars
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

uint64_t StepCursor::stepToNextSourceLine(pid_t pid, uint64_t addr)
{
	user_regs_struct regs;
	int wait_status;

	// Get registers
	ptrace(PTRACE_GETREGS, pid, 0, &regs);

	// Keep single stepping until a valid source line is reached and is
	// not the same line as the address passed in
	while (!isSourceLine(regs.rip) || regs.rip == addr)
	{
		// If we are stopped on a breakpoint, step over that first
		if (isBreakpointInstruction(pid, regs.rip - 1))
		{
			breakpoint_table->getBreakpoint(regs.rip - 1)->stepOver(pid);
		}

		// Single step and wait for completion
		if (ptrace(PTRACE_SINGLESTEP, pid, 0, 0))
		{
			perror("ptrace");
			return 0;
		}
		wait(&wait_status);

		// Get current address of instruction pointer
		ptrace(PTRACE_GETREGS, pid, 0, &regs);
	}
	return regs.rip;
}

bool StepCursor::isSourceLine(uint64_t addr)
{
	std::vector<Line> lines = debug_data->line()->getAllLines();
	for (auto line : lines)
	{
		if (line.address == addr)
		{
			return true;
		}
	}
	return false;
}

Line StepCursor::getLastLineOfSubprogram(DIESubprogram &subprogram)
{
	std::vector<Line> results;

	// Get all lines that lie in the address range of the specified subprogram
	std::vector<Line> lines = debug_data->line()->getAllLines();
	for (auto line : lines)
	{
		if (line.address >= subprogram.lowpc && line.address < (subprogram.lowpc + subprogram.highpc))
		{
			results.push_back(line);
		}
	}

	// Iterate over the results to find the last line by line number
	Line *last_line = &results[0];
	for (unsigned int i = 1; i < results.size(); i++)
	{
		if (results[i].number > last_line->number) last_line = &results[i];
	}

	return *last_line;
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
	return current_prog;
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

	procmsg("[STEP_CURSOR] Tracking vars updated: address = 0x%x, source = %s, line = %llu\n", address, source_file.c_str(), line_number);
}

bool StepCursor::isBreakpointInstruction(pid_t pid, uint64_t addr)
{
	uint64_t data = ptrace(PTRACE_PEEKTEXT, pid, addr, 0);
	procmsg("[IS_BREAKPOINT_INSTRUCTION] 0x%x: 0x%x\n", addr, data);
	return (data & 0xFF) == 0xCC;
}