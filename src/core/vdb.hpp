#pragma once

#include <sys/types.h>
#include <unistd.h>
#include <memory>

#include "dwarf/DwarfDebug.hpp"
#include "BreakpointTable.hpp"

class VDB
{
public:
	VDB();
	~VDB();

	bool init(const char *executable_name);
	bool run();

	std::shared_ptr<DwarfDebug> getDwarfDebugData();

	std::shared_ptr<BreakpointTable> breakpoint_table = nullptr;

private:

	bool runTarget();
	bool runDebugger();

	pid_t target_pid;
	char *target_name = NULL;

	std::shared_ptr<DwarfDebug> dwarf = nullptr;
};