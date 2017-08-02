#pragma once

#include "DebuggingInformationEntry.hpp"

#include <dwarf.h>
#include "libdwarf.h"

class DIESubrangeType : public DebuggingInformationEntry
{
public:
	DIESubrangeType(const Dwarf_Debug& dbg,
	                const Dwarf_Die &die,
	                DebuggingInformationEntry *parent) :
		DebuggingInformationEntry(dbg, die, parent)
	{
		
	}

	uint64_t getUpperBound();

private:
	uint64_t upper_bound;

	void onAttributeLoaded(const Dwarf_Attribute& attr,
	                       const Dwarf_Half &attr_code,
	                       const Dwarf_Half &form) override;
};