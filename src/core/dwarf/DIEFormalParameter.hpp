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

	// The length of the location data in bytes
	unsigned int location_data_length;
	void *location_data = nullptr;

	DIEFormalParameter(const Dwarf_Debug &dbg,
	                   const Dwarf_Die &die,
	                   DebuggingInformationEntry *parent) :
		DebuggingInformationEntry(dbg, die, parent)
	{

	}

private:
	virtual void onAttributeLoaded(const Dwarf_Attribute &attr,
	                               const Dwarf_Half &attr_code,
	                               const Dwarf_Half &form) override;
};