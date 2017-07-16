#include "BreakpointTable.hpp"

#include "dwarf/DebugLine.hpp"

#include <cstring>

BreakpointTable::BreakpointTable(std::shared_ptr<DwarfDebug> dwarf) :
	dwarf(dwarf)
{
	
}

void BreakpointTable::addBreakpoint(uint64_t address)
{
	mtx.lock();
	Breakpoint breakpoint((void *)address);
	std::pair<uint64_t, Breakpoint> entry(address, breakpoint);
	breakpoints_by_address.insert(entry);
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
	for (Line line : dwarf->line()->getAllLines())
	{
		if (strcmp(line.source, source_file) == 0 &&
		    line.number == line_number &&
		    breakpoints_by_address.find(line.address) == breakpoints_by_address.end())
		{
			Breakpoint breakpoint((void *)line.address, line.number, source_file);
			std::pair<uint64_t, Breakpoint> entry(line.address, breakpoint);
			breakpoints_by_address.insert(entry);

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
	for (Line line : dwarf->line()->getAllLines())
	{
		if (strcmp(line.source, source_file) == 0 &&
		    line.number == line_number &&
		    breakpoints_by_address.find(line.address) != breakpoints_by_address.end())
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

std::unique_ptr<Breakpoint> BreakpointTable::getBreakpoint(uint64_t address)
{
	mtx.lock();
	auto found = breakpoints_by_address.find(address);
	if (found == breakpoints_by_address.end())
	{
		mtx.unlock();
		return nullptr;
	}
	else
	{
		mtx.unlock();
		return std::make_unique<Breakpoint>(breakpoints_by_address.at(address));
	}
}