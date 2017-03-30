#include "DebuggingInformationEntry.hpp"

class DIEPointerType : public DebuggingInformationEntry
{
public:
	unsigned long long type_offset;
	unsigned long long byte_size;

	DIEPointerType(const Dwarf_Debug &dbg,
	               const Dwarf_Die &die,
	               DebuggingInformationEntry *parent) :
		DebuggingInformationEntry(dbg, die, parent)
	{

	}

private:
	virtual void onAttributeLoaded(const Dwarf_Attribute &attr,
	                               const Dwarf_Half &attr_code,
	                               const Dwarf_Half &form) override;
};