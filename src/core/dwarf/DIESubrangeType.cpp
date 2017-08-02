#include "DIESubrangeType.hpp"

void DIESubrangeType::onAttributeLoaded(const Dwarf_Attribute &attr,
                                        const Dwarf_Half &attr_code,
                                        const Dwarf_Half &form)
{
	switch (attr_code)
	{
	case DW_AT_upper_bound:
	{
		Dwarf_Unsigned value;
		dwarf_formudata(attr, &value, 0);
		upper_bound = value;
		procmsg("[DWARF] [DIESubrangeType] Saving attribute: DW_AT_upper_bound (%d)\n", value);
		break;
	}

	default:
		procmsg("[DWARF] [DIESubrangeType] Ignoring attribute...\n");
	}
}

uint64_t DIESubrangeType::getUpperBound()
{
	return upper_bound;
}