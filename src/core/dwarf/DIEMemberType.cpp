#include "DIEMemberType.hpp"

void DIEMemberType::onAttributeLoaded(const Dwarf_Attribute &attr,
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
		procmsg("[DWARF] [DIEMemberType] Saving attribute: DW_AT_name (%s)\n", cname);
		break;
	}

	case DW_AT_decl_line:
	{
		Dwarf_Unsigned value;
		dwarf_formudata(attr, &value, 0);
		decl_line_number = value;
		procmsg("[DWARF] [DIEMemberType] Saving attribute: DW_AT_decl_line (%d)\n", value);
		break;
	}

	case DW_AT_type:
	{
		Dwarf_Off type_offset = 0;
		dwarf_formref(attr, &type_offset, 0);
		this->type_offset = type_offset;
		procmsg("[DWARF] [DIEMemberType] Saving attribute: DW_AT_type (0x%llx)\n", type_offset);
		break;
	}

	case DW_AT_data_member_location:
	{
		Dwarf_Unsigned value;
		dwarf_formudata(attr, &value, 0);
		data_member_location = value;
		procmsg("[DWARF] [DIEMemberType] Saving attribute: DW_AT_data_member_location (%llu)\n", value);
		break;
	}

	default:
		procmsg("[DWARF] [DIEMemberType] Ignoring attribute...\n");
	}
}

std::string DIEMemberType::getName() const
{
	return name;
}

uint64_t DIEMemberType::getDeclLineNumber() const
{
	return decl_line_number;
}

uint64_t DIEMemberType::getTypeOffset() const
{
	return type_offset;
}

uint64_t DIEMemberType::getDataMemberLocation() const
{
	return data_member_location;
}