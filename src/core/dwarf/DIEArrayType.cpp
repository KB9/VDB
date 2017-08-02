#include "DIEArrayType.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

void DIEArrayType::onAttributeLoaded(const Dwarf_Attribute &attr,
                                     const Dwarf_Half &attr_code,
                                     const Dwarf_Half &form)
{
	switch (attr_code)
	{
	case DW_AT_type:
	{
		Dwarf_Off type_offset = 0;
		dwarf_formref(attr, &type_offset, 0);
		this->type_offset = type_offset;
		procmsg("[DWARF] [DIEArrayType] Saving attribute: DW_AT_type (0x%llx)\n", type_offset);
		break;
	}

	default:
		procmsg("[DWARF] [DIEArrayType] Ignoring attribute...\n");
	}
}

uint64_t DIEArrayType::getTypeOffset() const
{
	return type_offset;
}