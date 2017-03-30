#include "DIELexicalBlock.hpp"

void DIELexicalBlock::onAttributeLoaded(const Dwarf_Attribute &attr,
                                        const Dwarf_Half &attr_code,
                                        const Dwarf_Half &form)
{
	switch (attr_code)
	{
	case DW_AT_low_pc:
	{
		Dwarf_Addr addr;
		dwarf_formaddr(attr, &addr, 0);
		low_pc = addr;
		procmsg("[DWARF] [DIELexicalBlock] Saving attribute: DW_AT_low_pc (0x%08x)\n", low_pc);
		break;
	}

	case DW_AT_high_pc:
	{
		switch (form)
		{
		// DWARF 2 implementation
		case DW_FORM_addr:
		{
			Dwarf_Addr addr;
			dwarf_formaddr(attr, &addr, 0);
			high_pc = addr;
			break;
		}

		// DWARF 1 implementation
		case DW_FORM_data8:
		{
			Dwarf_Unsigned offset;
			dwarf_formudata(attr, &offset, 0);
			high_pc = offset;
			break;
		}
		}
		procmsg("[DWARF] [DIELexicalBlock] Saving attribute: DW_AT_high_pc (0x%08x)\n", high_pc);
		break;
	}

	default:
		procmsg("[DWARF] [DIELexicalBlock] Ignoring attribute...\n");
	}
}