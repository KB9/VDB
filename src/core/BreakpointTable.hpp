#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <memory>

#include "Breakpoint.hpp"
#include "dwarf/DwarfDebug.hpp"

class BreakpointTable
{
public:
	BreakpointTable(std::shared_ptr<DwarfDebug> dwarf);

	void addBreakpoint(uint64_t address);
	void removeBreakpoint(uint64_t address);
	std::unique_ptr<Breakpoint> getBreakpoint(uint64_t address);

	bool addBreakpoint(const char *source_file, unsigned int line_number);
	bool removeBreakpoint(const char *source_file, unsigned int line_number);

	void enableBreakpoints(pid_t target_pid);
	void disableBreakpoints(pid_t target_pid);

private:
	pid_t target_pid;
	std::shared_ptr<DwarfDebug> dwarf = nullptr;

	std::unordered_map<uint64_t, Breakpoint> breakpoints_by_address;
};