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
#include "ProcessTracer.hpp"

// This will improve upon the previous step cursor. Instead of checking each
// instruction to see if a breakpoint is on it, breakpoints will be utilised
// correctly. There will be no modification of the IP register. Instead,
// information will be deduced from the IP register.
class StepCursor
{
public:
	StepCursor(std::shared_ptr<DebugInfo> debug_info,
	           std::shared_ptr<BreakpointTable> user_breakpoints);

	void stepOver(ProcessTracer& tracer);
	void stepInto(ProcessTracer& tracer);
	void stepOut(ProcessTracer& tracer);

	uint64_t getCurrentAddress(ProcessTracer& tracer);
	uint64_t getCurrentLineNumber(ProcessTracer& tracer);
	std::string getCurrentSourceFile(ProcessTracer& tracer);

private:
	std::shared_ptr<DebugInfo> debug_info = nullptr;
	std::shared_ptr<BreakpointTable> user_breakpoints = nullptr;

	void addSubprogramBreakpoints(BreakpointTable &internal, ProcessTracer& tracer,
	                              uint64_t address);
	void addReturnBreakpoint(BreakpointTable &internal, ProcessTracer& tracer,
	                         uint64_t address);

	uint64_t getReturnAddress(pid_t pid);

	bool isStoppedAtUserBreakpoint(ProcessTracer& tracer);
	void stepOverUserBreakpoint(ProcessTracer& tracer);

	bool isCallInstruction(uint64_t address, ProcessTracer& tracer);

	bool hasHitBreakpoint(BreakpointTable &internal, ProcessTracer& tracer);

	void rewindIP(ProcessTracer& tracer);
};