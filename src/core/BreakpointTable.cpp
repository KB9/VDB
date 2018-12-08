#include "BreakpointTable.hpp"

#include <cstring>

BreakpointTable::BreakpointTable()
{
	
}

void BreakpointTable::addBreakpoint(uint64_t address)
{
	mtx.lock();
	Breakpoint breakpoint(address);
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

void BreakpointTable::enableBreakpoints(ProcessTracer& tracer)
{
	mtx.lock();
	for (auto it = breakpoints_by_address.begin();
	     it != breakpoints_by_address.end();
	     it++)
	{
		it->second.enable(tracer);
	}
	mtx.unlock();
}

void BreakpointTable::disableBreakpoints(ProcessTracer& tracer)
{
	mtx.lock();
	for (auto it = breakpoints_by_address.begin();
	     it != breakpoints_by_address.end();
	     it++)
	{
		it->second.disable(tracer);
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