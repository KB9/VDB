#include "DIEFormalParameter.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

void DIEFormalParameter::onAttributeLoaded(const Dwarf_Attribute &attr,
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
		procmsg("[DWARF] [DIEFormalParameter] Saving attribute: DW_AT_name (%s)\n", cname);
		break;
	}

	case DW_AT_decl_line:
	{
		Dwarf_Unsigned value;
		dwarf_formudata(attr, &value, 0);
		line_number = value;
		procmsg("[DWARF] [DIEFormalParameter] Saving attribute: DW_AT_decl_line (%d)\n", value);
		break;
	}
	}
}