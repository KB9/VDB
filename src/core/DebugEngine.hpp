#pragma once

#include "BreakpointTable.hpp"
#include "dwarf/DwarfDebug.hpp"
#include "ProcessDebugger.hpp"

#include <memory>

#include "ProcessDebugger.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

// Contains all of the tools needed to debug a process.
// Contains resources which can be used both in and outside of the debugging
// of a target process, such as setting breakpoints in preparation for
// debugging to begin.
class DebugEngine
{
public:
	DebugEngine(const char *executable_name, std::shared_ptr<DwarfDebug> debug_data);
	~DebugEngine();

	bool run();

	std::shared_ptr<BreakpointTable> getBreakpoints();

	void stepOver();
	void continueExecution();

	void sendMessage(std::unique_ptr<DebugMessage> msg);
	std::unique_ptr<DebugMessage> tryPoll();

	bool isDebugging();

private:
	char *target_name = NULL;

	std::shared_ptr<ProcessDebugger> debugger = nullptr;
	std::shared_ptr<DwarfDebug> debug_data = nullptr;
	std::shared_ptr<BreakpointTable> breakpoint_table = nullptr;
};