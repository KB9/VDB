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

	void sendMessage(std::unique_ptr<DebugMessage> msg)
	{
		debugger->enqueue(std::move(msg));
	}
	
	std::unique_ptr<DebugMessage> tryPoll()
	{
		return std::move(debugger->tryPoll());
	}

private:
	char *target_name = NULL;

	std::shared_ptr<ProcessDebugger> debugger = nullptr;
	std::shared_ptr<DwarfDebug> debug_data = nullptr;
	std::shared_ptr<BreakpointTable> breakpoint_table = nullptr;
};