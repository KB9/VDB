#include "BreakpointTable.hpp"

BreakpointTable::BreakpointTable(pid_t target_pid)
{
	this->target_pid = target_pid;
}

void BreakpointTable::addBreakpoint(uint64_t address)
{
	Breakpoint breakpoint(target_pid, (void *)address);
	std::pair<uint64_t, Breakpoint> entry(address, breakpoint);
	breakpoints_by_address.insert(entry);
}

std::unique_ptr<Breakpoint> BreakpointTable::getBreakpoint(uint64_t address)
{
	std::unordered_map<uint64_t, Breakpoint>::const_iterator found = breakpoints_by_address.find(address);
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