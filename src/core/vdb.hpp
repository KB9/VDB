#pragma once

#include <sys/types.h>
#include <unistd.h>
#include <memory>

#include "dwarf/DwarfDebug.hpp"
#include "BreakpointTable.hpp"

#include "DebugEngine.hpp"

class VDB
{
public:
	VDB();
	~VDB();

	bool init(const char *executable_name);
	bool isInitialized();

	std::shared_ptr<DwarfDebug> getDwarfDebugData();
	std::shared_ptr<DebugEngine> getDebugEngine();

private:
	bool is_initialized = false;

	std::shared_ptr<DwarfDebug> dwarf = nullptr;
	std::shared_ptr<DebugEngine> engine = nullptr;
};