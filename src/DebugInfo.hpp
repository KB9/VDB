#pragma once

#include <vector>
#include <memory>

#include "libdwarf.h"
#include <dwarf.h>

#include "CUHeader.hpp"

// Represents the information presented when performing a call to objdump --dwarf=info
class DebugInfo
{
public:
	DebugInfo(Dwarf_Debug dbg);

	std::vector<std::shared_ptr<CUHeader>> &getCUHeaders();
private:
	void loadCUHeaders(Dwarf_Debug dbg);
	
	std::vector<std::shared_ptr<CUHeader>> headers;
};