#pragma once

#include "DebuggingInformationEntry.hpp"

#include <dwarf.h>
#include "libdwarf.h"

class DIEArrayType : public DebuggingInformationEntry
{
public:
	DIEArrayType(const Dwarf_Debug& dbg,
	             const Dwarf_Die &die,
	             DebuggingInformationEntry *parent) :
		DebuggingInformationEntry(dbg, die, parent)
	{
		
	}

	uint64_t getTypeOffset() const;

private:
	uint64_t type_offset;

	void onAttributeLoaded(const Dwarf_Attribute& attr,
	                       const Dwarf_Half &attr_code,
	                       const Dwarf_Half &form) override;
};