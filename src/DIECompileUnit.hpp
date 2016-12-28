#pragma once

#include "DebuggingInformationEntry.hpp"

#include <dwarf.h>
#include "libdwarf.h"

// Compilation unit DIE
class DIECompileUnit : public DebuggingInformationEntry
{
public:
	DIECompileUnit(Dwarf_Debug dbg, Dwarf_Die die) :
		DebuggingInformationEntry(dbg, die)
	{

	}

private:
	virtual void onAttributeLoaded(Dwarf_Attribute attr, Dwarf_Half attr_code) override
	{

	}
};