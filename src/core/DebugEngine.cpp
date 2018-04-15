#include "DebugEngine.hpp"

DebugEngine::DebugEngine(const std::string& executable_name, std::shared_ptr<DebugInfo> debug_info) :
	target_name(executable_name),
	debug_info(debug_info)
{
	breakpoint_table = std::make_shared<BreakpointTable>(debug_info);
}

DebugEngine::~DebugEngine()
{
	
}

bool DebugEngine::run()
{
	debugger = std::make_shared<ProcessDebugger>(target_name, breakpoint_table, debug_info);
	return true;
}

std::shared_ptr<BreakpointTable> DebugEngine::getBreakpoints()
{
	return breakpoint_table;
}

void DebugEngine::stepOver()
{
	debugger->stepOver();
}

void DebugEngine::stepInto()
{
	debugger->stepInto();
}

void DebugEngine::stepOut()
{
	debugger->stepOut();
}

void DebugEngine::continueExecution()
{
	debugger->continueExecution();
}

void DebugEngine::sendMessage(std::unique_ptr<DebugMessage> msg)
{
	debugger->enqueue(std::move(msg));
}

std::unique_ptr<DebugMessage> DebugEngine::tryPoll()
{
	return std::move(debugger->tryPoll());
}

bool DebugEngine::isDebugging()
{
	return debugger != nullptr && debugger->isDebugging();
}