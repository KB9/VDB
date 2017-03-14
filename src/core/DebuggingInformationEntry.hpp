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
	DebuggingInformationEntry(const Dwarf_Debug &dbg,
	                          const Dwarf_Die &die,
	                          DebuggingInformationEntry *parent);
	virtual ~DebuggingInformationEntry() {}

	void init();

	// Get the parent DIE of this DIE
	DebuggingInformationEntry *getParent();

	// Get the child DIEs of this DIE
	std::vector<std::shared_ptr<DebuggingInformationEntry>> &getChildren();

	// Gets the tag name associated with this DIE
	std::string getTagName();

	Dwarf_Die &getInternalDie();

protected:
	virtual void onAttributeLoaded(const Dwarf_Attribute &attr,
	                               const Dwarf_Half &attr_code,
	                               const Dwarf_Half &form) = 0;
	
private:
	void loadAttributes();
	void loadChildren();
	void addChildDie(const Dwarf_Die &child_die);
	
	// Internal libdwarf type
	Dwarf_Die die;

	Dwarf_Debug dbg;

	// Parent DIE
	DebuggingInformationEntry *parent = nullptr;

	// Child DIEs
	std::vector<std::shared_ptr<DebuggingInformationEntry>> children;
};
