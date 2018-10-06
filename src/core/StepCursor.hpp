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
	StepCursor(std::shared_ptr<DebugInfo> debug_info,
	           std::shared_ptr<BreakpointTable> user_breakpoints);

	void stepOver(pid_t pid);
	void stepInto(pid_t pid);
	void stepOut(pid_t pid);

	uint64_t getCurrentAddress(pid_t pid);
	uint64_t getCurrentLineNumber(pid_t pid);
	std::string getCurrentSourceFile(pid_t pid);

private:
	std::shared_ptr<DebugInfo> debug_info = nullptr;
	std::shared_ptr<BreakpointTable> user_breakpoints = nullptr;

	void addSubprogramBreakpoints(pid_t pid, BreakpointTable &internal, uint64_t address);
	void addReturnBreakpoint(pid_t pid, BreakpointTable &internal, uint64_t address);

	uint64_t getReturnAddress(pid_t pid);

	bool isStoppedAtUserBreakpoint(pid_t pid);
	void stepOverUserBreakpoint(pid_t pid);

	bool isCallInstruction(pid_t pid, uint64_t address);

	bool hasHitBreakpoint(pid_t pid, BreakpointTable &internal);

	bool singleStep(pid_t pid);
	bool continueExec(pid_t pid);
};