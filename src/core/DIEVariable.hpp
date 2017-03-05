#include "DebuggingInformationEntry.hpp"

#include <dwarf.h>
#include "libdwarf.h"

#include <string>

class DIEVariable : public DebuggingInformationEntry
{
public:
	DIEVariable(const Dwarf_Debug &dbg,
	            const Dwarf_Die &die,
	            DebuggingInformationEntry *parent) :
		DebuggingInformationEntry(dbg, die, parent)
	{

	}

private:
	std::string name;
	unsigned int line_number;

	virtual void onAttributeLoaded(const Dwarf_Attribute &attr,
	                               const Dwarf_Half &attr_code,
	                               const Dwarf_Half &form) override;
};