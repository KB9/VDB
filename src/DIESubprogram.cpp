#include "DIESubprogram.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

void DIESubprogram::onAttributeLoaded(Dwarf_Attribute attr, Dwarf_Half attr_code)
{	
	switch (attr_code)
	{
	case DW_AT_name:
   {
      char *cname = 0;
      dwarf_formstring(attr, &cname, 0);
      name = cname;
      procmsg("[DWARF] [DIESubprogram] Saving attribute: DW_AT_name (%s)\n", cname);
		break;
   }

	case DW_AT_low_pc:
   {
      Dwarf_Addr addr;
		dwarf_formaddr(attr, &addr, 0);
      lowpc = addr;
      procmsg("[DWARF] [DIESubprogram] Saving attribute: DW_AT_low_pc (0x%08x)\n", lowpc);
		break;
   }

	case DW_AT_high_pc:
   {
      Dwarf_Signed svalue;
      dwarf_formsdata(attr, &svalue, 0);
		highpc = svalue;
      procmsg("[DWARF] [DIESubprogram] Saving attribute: DW_AT_high_pc (0x%08x)\n", highpc);
		break;
   }
   default:
      procmsg("[DWARF] [DIESubprogram] Ignoring attribute...\n");
	}
}
