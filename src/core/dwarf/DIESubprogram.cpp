#include "DIESubprogram.hpp"

#include <typeinfo>

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

std::vector<DIEFormalParameter> DIESubprogram::getFormalParameters()
{
   std::vector<DIEFormalParameter> parameters;
   for (auto &child : getChildren())
   {
      if (typeid(*child) == typeid(DIEFormalParameter))
      {
         std::shared_ptr<DIEFormalParameter> casted = std::static_pointer_cast<DIEFormalParameter>(child);
         parameters.push_back(*casted);
      }
   }
   return parameters;
}

void DIESubprogram::onAttributeLoaded(const Dwarf_Attribute &attr,
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

   case DW_AT_frame_base:
   {
      Dwarf_Unsigned expr_len = 0;
      Dwarf_Ptr block_ptr = NULL;
      dwarf_formexprloc(attr, &expr_len, &block_ptr, nullptr);
      frame_base_length = expr_len;
      frame_base_data = block_ptr;

      uint8_t *data = (uint8_t *)frame_base_data;
      procmsg("[DWARF] [DIESubprogram] Saving attribute: DW_AT_frame_base (0x%x: %d byte block)\n", data[0], expr_len);

      // data[0] is 0x9c, which is the code DW_OP_call_frame_cfa operation.
      // CFA stands for Canonical Frame Address, which is call frame address on the stack.
      // CFA defined to be value of stack pointer at call site in previous frame.
      //    (which may be different from its value on entry to the current frame)

      // procmsg("ATTEMPTING TO CREATE LOCLIST FROM EXPR...\n");
      // Dwarf_Locdesc *loc_desc = nullptr;
      // Dwarf_Signed list_len = 0;
      // dwarf_loclist_from_expr(dbg, block_ptr, expr_len, &loc_desc, &list_len, nullptr);
      // procmsg("Locdesc: ld_lopc=0x%llx ld_hipc=0x%llx ld_cents=%lu\n");

      // // ld_cents tells how many entries ld_s points to. If 0, don't read.
      // procmsg("RETRIEVING LOC...\n");
      // Dwarf_Loc *loc = loc_desc->ld_s;
      // procmsg("Loc: lr_atom=%x lr_number=%lu lr_number2=%lu lr_offset=%lx\n");
      // break;
   }
   
   default:
      procmsg("[DWARF] [DIESubprogram] Ignoring attribute...\n");
	}
}
