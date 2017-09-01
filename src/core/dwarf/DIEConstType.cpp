#include "DIEConstType.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

void DIEConstType::onAttributeLoaded(const Dwarf_Attribute &attr,
                                     const Dwarf_Half &attr_code,
                                     const Dwarf_Half &form)
{
	switch (attr_code)
	{
	case DW_AT_type:
	{
		Dwarf_Off offset = 0;
		dwarf_formref(attr, &offset, 0);
		type_offset = offset;
		procmsg("[DWARF] [DIEConstType] Saving attribute: DW_AT_type (%llu)\n", offset);
		break;
	}

	default:
		procmsg("[DWARF] [DIEConstType] Ignoring attribute...\n");
	}
}

uint64_t DIEConstType::getTypeOffset() const
{
	return type_offset;
}