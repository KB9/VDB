#include "DIEBaseType.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

void DIEBaseType::onAttributeLoaded(const Dwarf_Attribute &attr,
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
		procmsg("[DWARF] [DIEBaseType] Saving attribute: DW_AT_name (%s)\n", cname);
		break;
	}

	case DW_AT_encoding:
	{
		Dwarf_Unsigned enc;
		dwarf_formudata(attr, &enc, 0);
		encoding = enc;
		procmsg("[DWARF] [DIEBaseType] Saving attribute: DW_AT_encoding (%d: %s)\n", encoding, getEncodingMeaning());
		break;
	}

	case DW_AT_byte_size:
	{
		Dwarf_Unsigned size;
		dwarf_formudata(attr, &size, 0);
		byte_size = size;
		procmsg("[DWARF] [DIEBaseType] Saving attribute: DW_AT_byte_size (%d)\n", byte_size);
		break;
	}

	default:
		procmsg("[DWARF] [DIEBaseType] Ignoring attribute...\n");
	}
}

unsigned int DIEBaseType::getEncoding() const
{
	return encoding;
}

const char *DIEBaseType::getEncodingMeaning()
{
	switch (encoding)
	{
	case DW_ATE_address:		return "linear machine address";
	case DW_ATE_boolean:		return "true or false";
	case DW_ATE_complex_float:	return "complex binary floating-point number";
	case DW_ATE_float:			return "binary floating-point number";
	case DW_ATE_imaginary_float:return "imaginary binary floating-point number";
	case DW_ATE_signed:			return "signed binary integer";
	case DW_ATE_signed_char:	return "signed character";
	case DW_ATE_unsigned:		return "unsigned binary integer";
	case DW_ATE_unsigned_char:	return "unsigned character";
	case DW_ATE_packed_decimal:	return "packed decimal";
	case DW_ATE_numeric_string:	return "numeric string";
	case DW_ATE_edited:			return "edited string";
	case DW_ATE_signed_fixed:	return "signed fixed-point scaled integer";
	case DW_ATE_unsigned_fixed:	return "unsigned fixed-point scaled integer";
	case DW_ATE_decimal_float:	return "decimal floating-point number";
	default:					return "unknown";
	}
}

unsigned int DIEBaseType::getByteSize() const
{
	return byte_size;
}