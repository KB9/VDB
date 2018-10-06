#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <memory>

#include <mutex>

#include "Breakpoint.hpp"
#include "DebugInfo.hpp"

class BreakpointTable
{
public:
	BreakpointTable(std::shared_ptr<DebugInfo> debug_info);

	void addBreakpoint(uint64_t address);
	void removeBreakpoint(uint64_t address);
	Breakpoint &getBreakpoint(uint64_t address);

	bool addBreakpoint(const char *source_file, unsigned int line_number);
	bool removeBreakpoint(const char *source_file, unsigned int line_number);

	void enableBreakpoints(pid_t target_pid);
	void disableBreakpoints(pid_t target_pid);

	bool isBreakpoint(uint64_t address);
	bool isBreakpoint(const std::string &source_file, unsigned int line_number);

private:
	std::shared_ptr<DebugInfo> debug_info = nullptr;

	std::mutex mtx;

	std::unordered_map<uint64_t, Breakpoint> breakpoints_by_address;
};