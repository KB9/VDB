#include "BreakpointTable.hpp"

#include "DebugLine.hpp"

#include <cstring>

BreakpointTable::BreakpointTable(DwarfDebug &dwarf) :
	dwarf(dwarf)
{
	
}

void BreakpointTable::addBreakpoint(uint64_t address)
{
	Breakpoint breakpoint((void *)address);
	std::pair<uint64_t, Breakpoint> entry(address, breakpoint);
	breakpoints_by_address.insert(entry);
}



bool BreakpointTable::addBreakpoint(const char *source_file,
                                    unsigned int line_number)
{
	for (Line line : dwarf.line()->getAllLines())
	{
		if (strcmp(line.source, source_file) == 0 &&
		    line.number == line_number &&
		    breakpoints_by_address.find(line.address) == breakpoints_by_address.end())
		{
			addBreakpoint(line.address);
			return true;
		}
	}
	return false;
}

bool BreakpointTable::removeBreakpoint(const char *source_file,
                                       unsigned int line_number)
{
	for (Line line : dwarf.line()->getAllLines())
	{
		if (strcmp(line.source, source_file) == 0 &&
		    line.number == line_number &&
		    breakpoints_by_address.find(line.address) != breakpoints_by_address.end())
		{
			removeBreakpoint(line.address);
			return true;
		}
	}
	return false;
}

void BreakpointTable::enableBreakpoints(pid_t target_pid)
{
	for (auto it = breakpoints_by_address.begin();
	     it != breakpoints_by_address.end();
	     it++)
	{
		it->second.enable(target_pid);
	}
}

void BreakpointTable::disableBreakpoints(pid_t target_pid)
{
	for (auto it = breakpoints_by_address.begin();
	     it != breakpoints_by_address.end();
	     it++)
	{
		it->second.disable(target_pid);
	}
}

std::unique_ptr<Breakpoint> BreakpointTable::getBreakpoint(uint64_t address)
{
	auto found = breakpoints_by_address.find(address);
	if (found == breakpoints_by_address.end())
	{
		return nullptr;
	}
	else
	{
		return std::make_unique<Breakpoint>(breakpoints_by_address.at(address));
	}
}

void BreakpointTable::removeBreakpoint(uint64_t address)
{
	breakpoints_by_address.erase(address);
}