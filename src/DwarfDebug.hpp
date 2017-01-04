#pragma once

#include <memory>
#include <vector>

#include <dwarf.h>
#include "libdwarf.h"

#include <string>
#include <cstring>

#include "DebugInfo.hpp"
#include "DebugLine.hpp"

class DwarfDebug
{
public:
	DwarfDebug(std::string filename);
	~DwarfDebug();

	std::shared_ptr<DebugInfo> info();
	std::shared_ptr<DebugLine> line();

private:
	FILE *file;

	Dwarf_Debug dbg;

	std::shared_ptr<DebugInfo> debug_info = nullptr;
	std::shared_ptr<DebugLine> debug_line = nullptr;
};