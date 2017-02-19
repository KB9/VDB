#pragma once

#include <sys/types.h>
#include <unistd.h>
#include <memory>

#include "DwarfDebug.hpp"

class VDB
{
public:
	VDB();
	~VDB();

	bool run(const char *executable_name);

	std::shared_ptr<DwarfDebug> getDwarfDebugData()
	{
		return dwarf;
	}

private:
	bool runTarget(const char *executable_name);
	bool runDebugger(pid_t child_pid, const char *child_name);

	std::shared_ptr<DwarfDebug> dwarf = nullptr;
};