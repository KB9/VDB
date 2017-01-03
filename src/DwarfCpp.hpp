#pragma once

#include <memory>
#include <vector>

#include <dwarf.h>
#include "libdwarf.h"

#include <string>

#include "CUHeader.hpp"

// Represents the information presented when performing a call to objdump --dwarf=info
class DebugInfo
{
public:
	DebugInfo();

	std::vector<std::unique_ptr<CUHeader>> &getCUHeaders();
private:
	void loadCUHeaders(Dwarf_Debug dbg);
	
	std::vector<std::unique_ptr<CUHeader>> headers;
};