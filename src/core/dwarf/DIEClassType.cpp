#include "DIEClassType.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

void DIEClassType::onAttributeLoaded(const Dwarf_Attribute &attr,
                                     const Dwarf_Half &attr_code,
                                     const Dwarf_Half &form)
{
	switch (attr_code)
	{
	case DW_AT_name:
	{
		char *cname = 0;
		dwarf_formstring(attr, &cname, 0);
		name = cname;
		procmsg("[DWARF] [DIEClassType] Saving attribute: DW_AT_name (%s)\n", cname);
		break;
	}

	case DW_AT_decl_line:
	{
		Dwarf_Unsigned value;
		dwarf_formudata(attr, &value, 0);
		decl_line_number = value;
		procmsg("[DWARF] [DIEClassType] Saving attribute: DW_AT_decl_line (%d)\n", value);
		break;
	}

	case DW_AT_byte_size:
	{
		Dwarf_Unsigned size;
		dwarf_formudata(attr, &size, 0);
		byte_size = size;
		procmsg("[DWARF] [DIEClassType] Saving attribute: DW_AT_byte_size (%d)\n", byte_size);
		break;
	}

	default:
		procmsg("[DWARF] [DIEClassType] Ignoring attribute...\n");
	}
}

std::string DIEClassType::getName() const
{
	return name;
}

uint64_t DIEClassType::getDeclLineNumber() const
{
	return decl_line_number;
}

uint64_t DIEClassType::getByteSize() const
{
	return byte_size;
}