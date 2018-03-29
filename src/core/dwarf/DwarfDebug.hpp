#pragma once

#include <memory>
#include <vector>

#include <dwarf.h>
#include "libdwarf.h"

#include <string>
#include <cstring>

// #include "DebugInfo.hpp"
#include "DwarfReader.hpp"
#include "DebugLine.hpp"
#include "DebugAddressRanges.hpp"

class DwarfDebug
{
public:
	DwarfDebug(std::string filename);
	~DwarfDebug();

	// std::shared_ptr<DebugInfo> info();
	std::shared_ptr<DwarfInfoReader> info();
	std::shared_ptr<DebugLine> line();
	std::shared_ptr<DebugAddressRanges> aranges();

private:
	FILE *file;

	Dwarf_Debug dbg;

	// std::shared_ptr<DebugInfo> debug_info = nullptr;
	std::shared_ptr<DwarfInfoReader> debug_info = nullptr;
	std::shared_ptr<DebugLine> debug_line = nullptr;
	std::shared_ptr<DebugAddressRanges> debug_aranges = nullptr;
};