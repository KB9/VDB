#include "DIESubprogram.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

void DIESubprogram::onAttributeLoaded(Dwarf_Attribute attr, Dwarf_Half attr_code, Dwarf_Half form)
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
      switch (form)
      {
      // DWARF 2 implementation
      case DW_FORM_addr:
      {
         Dwarf_Addr addr;
         dwarf_formaddr(attr, &addr, 0);
         highpc = addr;
         break;
      }

      // DWARF 1 implementation
      case DW_FORM_data8:
      {
         Dwarf_Unsigned offset;
         dwarf_formudata(attr, &offset, 0);
         highpc = offset;
         break;
      }
      }
      procmsg("[DWARF] [DIESubprogram] Saving attribute: DW_AT_high_pc (0x%08x)\n", highpc);
		break;
   }

   case DW_AT_decl_line:
   {
      Dwarf_Unsigned value;
      dwarf_formudata(attr, &value, 0);
      line_number = value;
      procmsg("[DWARF] [DIESubprogram] Saving attribute: DW_AT_decl_line (%d)\n", value);
      break;
   }
   
   default:
      procmsg("[DWARF] [DIESubprogram] Ignoring attribute...\n");
	}
}
