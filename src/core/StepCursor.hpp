#pragma once

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>

#include <unistd.h>
#include <assert.h>
#include <string>
#include <vector>

#include "DebugInfo.hpp"
#include "BreakpointTable.hpp"

// This will improve upon the previous step cursor. Instead of checking each
// instruction to see if a breakpoint is on it, breakpoints will be utilised
// correctly. There will be no modification of the IP register. Instead,
// information will be deduced from the IP register.
class StepCursor
{
public:
	StepCursor(pid_t pid,
	           std::shared_ptr<DebugInfo> debug_info,
	           std::shared_ptr<BreakpointTable> user_breakpoints);

	void stepOver();
	void stepInto();
	void stepOut();

	uint64_t getCurrentAddress();
	uint64_t getCurrentLineNumber();
	std::string getCurrentSourceFile();

private:
	pid_t pid;
	std::shared_ptr<DebugInfo> debug_info = nullptr;
	std::shared_ptr<BreakpointTable> user_breakpoints = nullptr;

	void addSubprogramBreakpoints(BreakpointTable &internal, uint64_t address);
	void addReturnBreakpoint(BreakpointTable &internal, uint64_t address);

	uint64_t getReturnAddress();

	bool isStoppedAtUserBreakpoint();
	void stepOverUserBreakpoint();

	bool isCallInstruction(uint64_t address);

	bool hasHitBreakpoint(BreakpointTable &internal);

	bool singleStep();
	bool continueExec();
};