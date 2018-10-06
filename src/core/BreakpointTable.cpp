#include "BreakpointTable.hpp"

#include <cstring>

BreakpointTable::BreakpointTable(std::shared_ptr<DebugInfo> debug_info) :
	debug_info(debug_info)
{
	
}

void BreakpointTable::addBreakpoint(uint64_t address)
{
	mtx.lock();
	Breakpoint breakpoint((void *)address);
	auto pair = std::make_pair(address, std::move(breakpoint));
	breakpoints_by_address.insert(std::move(pair));
	mtx.unlock();
}

void BreakpointTable::removeBreakpoint(uint64_t address)
{
	mtx.lock();
	breakpoints_by_address.erase(address);
	mtx.unlock();
}

bool BreakpointTable::addBreakpoint(const char *source_file,
                                    unsigned int line_number)
{
	mtx.lock();
	for (const DebugInfo::SourceLine &line : debug_info->getSourceFileLines(source_file))
	{
		bool is_match = line.file_name == source_file && line.number == line_number;
		bool is_not_breakpoint = breakpoints_by_address.find(line.address) == breakpoints_by_address.end();
		if (is_match && is_not_breakpoint)
		{
			Breakpoint breakpoint((void *)line.address, line.number, source_file);
			auto pair = std::make_pair(line.address, std::move(breakpoint));
			breakpoints_by_address.insert(std::move(pair));

			mtx.unlock();
			return true;
		}
	}
	mtx.unlock();
	return false;
}

bool BreakpointTable::removeBreakpoint(const char *source_file,
                                       unsigned int line_number)
{
	mtx.lock();
	for (const DebugInfo::SourceLine &line : debug_info->getSourceFileLines(source_file))
	{
		bool is_match = line.file_name == source_file && line.number == line_number;
		bool is_breakpoint = breakpoints_by_address.find(line.address) != breakpoints_by_address.end();
		if (is_match && is_breakpoint)
		{
			breakpoints_by_address.erase(line.address);

			mtx.unlock();
			return true;
		}
	}
	mtx.unlock();
	return false;
}

void BreakpointTable::enableBreakpoints(pid_t target_pid)
{
	mtx.lock();
	for (auto it = breakpoints_by_address.begin();
	     it != breakpoints_by_address.end();
	     it++)
	{
		it->second.enable(target_pid);
	}
	mtx.unlock();
}

void BreakpointTable::disableBreakpoints(pid_t target_pid)
{
	mtx.lock();
	for (auto it = breakpoints_by_address.begin();
	     it != breakpoints_by_address.end();
	     it++)
	{
		it->second.disable(target_pid);
	}
	mtx.unlock();
}

Breakpoint &BreakpointTable::getBreakpoint(uint64_t address)
{
	return breakpoints_by_address.at(address);
}

bool BreakpointTable::isBreakpoint(uint64_t address)
{
	auto it = breakpoints_by_address.find(address);
	return it != breakpoints_by_address.end();
}

bool BreakpointTable::isBreakpoint(const std::string &source_file,
                                   unsigned int line_number)
{
	for (const DebugInfo::SourceLine &line : debug_info->getSourceFileLines(source_file))
	{
		bool is_match = line.file_name == source_file && line.number == line_number;
		bool is_breakpoint = breakpoints_by_address.find(line.address) != breakpoints_by_address.end();
		if (is_match && is_breakpoint)
		{
			return true;
		}
	}
	return false;
}