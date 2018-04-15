#pragma once

#include <sys/types.h>
#include <unistd.h>
#include <memory>

#include "DebugInfo.hpp"
#include "BreakpointTable.hpp"

#include "DebugEngine.hpp"

class VDB
{
public:
	VDB();
	~VDB();

	bool init(const char *executable_name);
	bool isInitialized();

	std::shared_ptr<DebugInfo> getDebugInfo();
	std::shared_ptr<DebugEngine> getDebugEngine();

private:
	bool is_initialized = false;

	bool isExecutableFile(const char *executable_name);

	std::shared_ptr<DebugInfo> debug_info = nullptr;
	std::shared_ptr<DebugEngine> engine = nullptr;
};