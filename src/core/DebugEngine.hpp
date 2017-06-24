#pragma once

#include "BreakpointTable.hpp"
#include "dwarf/DwarfDebug.hpp"
#include "ProcessDebugger.hpp"

#include <memory>

#include "ProcessDebugger.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

// Contains all of the tools needed to debug a process
class DebugEngine
{
public:
	DebugEngine(const char *executable_name, std::shared_ptr<DwarfDebug> debug_data);
	~DebugEngine();

	bool run(BreakpointCallback breakpoint_callback);

	std::shared_ptr<BreakpointTable> getBreakpoints();

	char *getValue(const char *variable_name);

private:
	char *target_name = NULL;

	std::shared_ptr<ProcessDebugger> debugger = nullptr;
	std::shared_ptr<DwarfDebug> debug_data = nullptr;
	std::shared_ptr<BreakpointTable> breakpoint_table = nullptr;
};