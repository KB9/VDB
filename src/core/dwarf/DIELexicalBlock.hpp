#pragma once

#include "DebuggingInformationEntry.hpp"

#include <dwarf.h>
#include "libdwarf.h"

#include <stdint.h>

class DIELexicalBlock : public DebuggingInformationEntry
{
public:
	DIELexicalBlock(const Dwarf_Debug &dbg,
	                const Dwarf_Die &die,
	                DebuggingInformationEntry *parent) :
		DebuggingInformationEntry(dbg, die, parent)
	{

	}

private:
	unsigned low_pc;
	unsigned high_pc;

	virtual void onAttributeLoaded(const Dwarf_Attribute &attr,
	                               const Dwarf_Half &attr_code,
	                               const Dwarf_Half &form) override;
};