#pragma once

#include <dwarf.h>
#include "libdwarf.h"

#include <memory>

#include "DebuggingInformationEntry.hpp"
#include "DIECompileUnit.hpp"

class CUHeader
{
public:
	CUHeader(Dwarf_Debug dbg);

	std::unique_ptr<DIECompileUnit> root_die;
	
	Dwarf_Unsigned length;
	Dwarf_Half version_stamp;
	Dwarf_Unsigned abbrev_offset;
	Dwarf_Half address_size;
	Dwarf_Unsigned next_cu_header;
};
