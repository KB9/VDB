#pragma once

#include "DebuggingInformationEntry.hpp"

#include <dwarf.h>
#include "libdwarf.h"

#include <string>

// Represents a formal parameter in a function's arguments
class DIEFormalParameter : public DebuggingInformationEntry
{
public:
	// The name assigned to this formal parameter
	std::string name;

	// The line number this formal parameter is defined on
	unsigned int line_number;

	DIEFormalParameter(Dwarf_Debug dbg, Dwarf_Die die) :
		DebuggingInformationEntry(dbg, die)
	{

	}

private:
	virtual void onAttributeLoaded(Dwarf_Attribute attr, Dwarf_Half attr_code, Dwarf_Half form) override;
};