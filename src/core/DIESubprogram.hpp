#pragma once

#include "DebuggingInformationEntry.hpp"

#include "DIEFormalParameter.hpp"

#include <dwarf.h>
#include "libdwarf.h"

#include <string>
#include <vector>

// Function DIE
class DIESubprogram : public DebuggingInformationEntry
{
public:
	// Function name
	std::string name;

	// Function entry address
	unsigned int lowpc;
	
	// DWARF: The size of the function
	// DWARF 2: The function end address
	unsigned int highpc;

	// The line number the function is implemented at
	unsigned int line_number;

	DIESubprogram(const Dwarf_Debug &dbg, const Dwarf_Die &die) :
		DebuggingInformationEntry(dbg, die)
	{
	    
	}

	// Returns a vector of the formal parameters applied to this subprogram
	std::vector<DIEFormalParameter> getFormalParameters();

private:
	virtual void onAttributeLoaded(const Dwarf_Attribute &attr,
	                               const Dwarf_Half &attr_code,
	                               const Dwarf_Half &form) override;
};
