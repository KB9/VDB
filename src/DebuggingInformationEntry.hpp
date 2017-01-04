#pragma once

#include <dwarf.h>
#include "libdwarf.h"

#include <string>
#include <vector>
#include <memory>

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

class DebuggingInformationEntry
{
public:
	DebuggingInformationEntry(Dwarf_Debug dbg, Dwarf_Die die);

	// Get the child DIEs of this DIE
	std::vector<std::shared_ptr<DebuggingInformationEntry>> &getChildren();

	// Gets the tag name associated with this DIE
	std::string getTagName();

	// NOTE: Had to make this callable externally as PV overriden methods can't be called in constructor/destructor
	void loadAttributes();

	Dwarf_Die &getInternalDie();

protected:
	virtual void onAttributeLoaded(Dwarf_Attribute attr, Dwarf_Half attr_code, Dwarf_Half form) = 0;
	
private:
	void loadChildren();
	
	// Internal libdwarf type
	Dwarf_Die die;

	Dwarf_Debug dbg;

	// Child DIEs
	std::vector<std::shared_ptr<DebuggingInformationEntry>> children;
};
