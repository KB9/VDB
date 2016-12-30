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
	std::vector<std::unique_ptr<DebuggingInformationEntry>> &getChildren();

	// Gets the tag name associated with this DIE
	std::string getTagName(Dwarf_Debug dbg);

	// NOTE: Had to make this callable externally as PV overriden methods can't be called in constructor/destructor
	void loadAttributes();

protected:
	virtual void onAttributeLoaded(Dwarf_Attribute attr, Dwarf_Half attr_code, Dwarf_Half form) = 0;
	
private:
	void loadChildren(Dwarf_Debug dbg);
	//void loadAttributes();
	
	// Internal libdwarf type
	Dwarf_Die die;

	// Child DIEs
	std::vector<std::unique_ptr<DebuggingInformationEntry>> children;
};
