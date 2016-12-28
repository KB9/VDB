#pragma once

#include "DebuggingInformationEntry.hpp"

#include <dwarf.h>
#include "libdwarf.h"

#include <string>

// Function DIE
class DIESubprogram : public DebuggingInformationEntry
{
public:
	DIESubprogram(Dwarf_Debug dbg, Dwarf_Die die) :
		DebuggingInformationEntry(dbg, die)
	{
	    
	}

private:
	virtual void onAttributeLoaded(Dwarf_Attribute attr, Dwarf_Half attr_code) override;

public:
	// Function name
	std::string name;

	// Function entry address
	unsigned int lowpc;
	
	unsigned int highpc;
};
