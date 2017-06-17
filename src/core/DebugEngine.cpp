#include "DebugEngine.hpp"

DebugEngine::DebugEngine(const char *executable_name, std::shared_ptr<DwarfDebug> debug_data) :
	debug_data(debug_data)
{
	if (target_name != NULL) delete target_name;
	target_name = new char[strlen(executable_name) + 1];
	strcpy(target_name, executable_name);

	breakpoint_table = std::make_shared<BreakpointTable>(debug_data);
}

DebugEngine::~DebugEngine()
{
	if (target_name != NULL) delete target_name;
	target_name = NULL;
}

bool DebugEngine::run(BreakpointCallback breakpoint_callback)
{
	debugger = std::make_shared<ProcessDebugger>(target_name, breakpoint_table, breakpoint_callback);
	return true;
}

std::shared_ptr<BreakpointTable> DebugEngine::getBreakpoints()
{
	return breakpoint_table;
}