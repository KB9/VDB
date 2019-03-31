#include "DebugEngine.hpp"

DebugEngine::DebugEngine(const std::string& executable_name, std::shared_ptr<ELFFile> elf_info) :
	target_name(executable_name),
	elf_info(elf_info)
{

}

DebugEngine::~DebugEngine()
{
	
}

bool DebugEngine::run()
{
	debugger = std::make_shared<ProcessDebugger>(target_name, breakpoint_lines, elf_info);
	return true;
}

bool DebugEngine::addBreakpoint(const char* source_file, unsigned int line_number)
{
	BreakpointLine line;
	line.line_number = line_number;
	line.file_name = source_file;

	auto it = std::find(std::begin(breakpoint_lines), std::end(breakpoint_lines), line);
	bool does_not_exist = it == std::end(breakpoint_lines);
	if (does_not_exist)
	{
		breakpoint_lines.push_back(line);
		return true;
	}
	else
	{
		return false;
	}
}

bool DebugEngine::removeBreakpoint(const char* source_file, unsigned int line_number)
{
	BreakpointLine line;
	line.line_number = line_number;
	line.file_name = source_file;

	auto it = std::find(std::begin(breakpoint_lines), std::end(breakpoint_lines), line);
	bool does_exist = it != std::end(breakpoint_lines);
	if (does_exist)
	{
		breakpoint_lines.erase(it);
		return true;
	}
	else
	{
		return false;
	}
}

bool DebugEngine::isBreakpoint(const char* source_file, unsigned int line_number)
{
	BreakpointLine line;
	line.line_number = line_number;
	line.file_name = source_file;

	auto it = std::find(std::begin(breakpoint_lines), std::end(breakpoint_lines), line);
	bool does_exist = it != std::end(breakpoint_lines);
	return does_exist;
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