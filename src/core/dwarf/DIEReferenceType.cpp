#include "DIEReferenceType.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

void DIEReferenceType::onAttributeLoaded(const Dwarf_Attribute &attr,
                                         const Dwarf_Half &attr_code,
                                         const Dwarf_Half &form)
{
	switch (attr_code)
	{
	case DW_AT_byte_size:
	{
		Dwarf_Unsigned value = 0;
		dwarf_formudata(attr, &value, 0);
		byte_size = value;
		procmsg("[DWARF] [DIEReferenceType] Saving attribute: DW_AT_byte_size (%llu)\n", value);
		break;
	}

	case DW_AT_type:
	{
		Dwarf_Off offset = 0;
		dwarf_formref(attr, &offset, 0);
		type_offset = offset;
		procmsg("[DWARF] [DIEReferenceType] Saving attribute: DW_AT_type (%llu)\n", offset);
		break;
	}

	default:
		procmsg("[DWARF] [DIEReferenceType] Ignoring attribute...\n");
	}
}