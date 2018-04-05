#pragma once

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>

#include <unistd.h>
#include <assert.h>
#include <string>
#include <vector>

#include "dwarf/DebugLine.hpp"
#include "dwarf/DwarfDebug.hpp"
#include "BreakpointTable.hpp"

// Provides functionality for source-level stepping including stepping over
// lines, stepping into functions and stepping out of functions.
class StepCursor
{
public:
	StepCursor(uint64_t address, std::shared_ptr<DwarfDebug> debug_data,
	           std::shared_ptr<BreakpointTable> user_breakpoints);

	void stepOver(pid_t pid);
	void stepInto(pid_t pid);
	void stepOut(pid_t pid);

	uint64_t getCurrentAddress();
	uint64_t getCurrentLineNumber();
	std::string getCurrentSourceFile();

private:
	std::shared_ptr<DwarfDebug> debug_data = nullptr;
	std::shared_ptr<BreakpointTable> user_breakpoints = nullptr;

	// Updated after every step to keep track of the cursor
	uint64_t address = 0;
	uint64_t line_number;
	std::string source_file;

	uint64_t stepToNextSourceLine(pid_t pid, uint64_t addr,
	                              bool include_current_addr = false);
	uint64_t stepToCallingFunction(pid_t pid, uint64_t addr);
	DIE getSubprogramFromAddress(uint64_t address);
	std::unique_ptr<BreakpointTable> createSubprogramBreakpoints(pid_t pid,
	                                                             uint64_t addr,
	                                                             bool include_current_addr);
	std::unique_ptr<BreakpointTable> createReturnBreakpoint(pid_t pid,
	                                                        uint64_t addr);
	void updateTrackingVars(uint64_t addr);
};