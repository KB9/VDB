#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <memory>

#include <mutex>

#include "Breakpoint.hpp"
#include "ProcessTracer.hpp"

class BreakpointTable
{
public:
	BreakpointTable();

	void addBreakpoint(uint64_t address);
	void removeBreakpoint(uint64_t address);
	Breakpoint &getBreakpoint(uint64_t address);

	void enableBreakpoints(ProcessTracer& tracer);
	void disableBreakpoints(ProcessTracer& tracer);

	bool isBreakpoint(uint64_t address);

private:
	std::mutex mtx;
	std::unordered_map<uint64_t, Breakpoint> breakpoints_by_address;
};