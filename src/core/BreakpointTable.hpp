#pragma once

#include <unordered_map>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <memory>

#include "Breakpoint.hpp"

class BreakpointTable
{
public:
	BreakpointTable(pid_t target_pid);

	void addBreakpoint(uint64_t address);
	std::unique_ptr<Breakpoint> getBreakpoint(uint64_t address);
	void removeBreakpoint(uint64_t address);

private:
	pid_t target_pid;
	std::unordered_map<uint64_t, Breakpoint> breakpoints_by_address;
};