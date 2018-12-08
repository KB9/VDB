#pragma once

#include "BreakpointTable.hpp"
#include "DebugInfo.hpp"
#include "ProcessDebugger.hpp"

#include <memory>
#include <string>

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
	DebugEngine(const std::string& executable_name, std::shared_ptr<DebugInfo> debug_info);
	~DebugEngine();

	bool run();

	bool addBreakpoint(const char* source_file, unsigned int line_number);
	bool removeBreakpoint(const char* source_file, unsigned int line_number);
	bool isBreakpoint(const char* source_file, unsigned int line_number);

	void stepOver();
	void stepInto();
	void stepOut();
	void continueExecution();

	void sendMessage(std::unique_ptr<DebugMessage> msg);
	std::unique_ptr<DebugMessage> tryPoll();

	bool isDebugging();

private:
	std::string target_name;

	std::shared_ptr<ProcessDebugger> debugger = nullptr;
	std::shared_ptr<DebugInfo> debug_info = nullptr;
	std::vector<BreakpointLine> breakpoint_lines;
};