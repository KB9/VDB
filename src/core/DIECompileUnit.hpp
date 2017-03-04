#pragma once

#include "DebuggingInformationEntry.hpp"

#include <dwarf.h>
#include "libdwarf.h"

#include <string>

// Compilation unit DIE
class DIECompileUnit : public DebuggingInformationEntry
{
public:
	DIECompileUnit(const Dwarf_Debug &dbg, const Dwarf_Die &die) :
		DebuggingInformationEntry(dbg, die)
	{

	}

	std::string getName();
	std::string getCompDir();

private:
	virtual void onAttributeLoaded(const Dwarf_Attribute &attr,
	                               const Dwarf_Half &attr_code,
	                               const Dwarf_Half &form) override;

	// The name (filename) of the compilation unit
	std::string name;

	// The directory of the compilation unit file
	std::string comp_dir;

	// Comilation unit entry address
	unsigned int lowpc;

	unsigned int highpc;
};