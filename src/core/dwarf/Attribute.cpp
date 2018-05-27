#include "Attribute.hpp"

#include <cassert>

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

template <Dwarf_Half CODE>
bool Attribute<CODE>::isMatchingType(const Dwarf_Attribute &attr)
{
	Dwarf_Half code;
	Dwarf_Error err;
	bool whatattr_success = (dwarf_whatattr(attr, &code, &err) == DW_DLV_OK);
	bool code_matches = (code == CODE);
	return whatattr_success && code_matches;
}

template <Dwarf_Half CODE>
typename Attribute<CODE>::value_type Attribute<CODE>::value(const Dwarf_Attribute &attr)
{

}

template <>
Attribute<DW_AT_type>::value_type Attribute<DW_AT_type>::value(const Dwarf_Attribute &attr)
{
	assert(isMatchingType(attr) && "Dwarf_Attribute code doesn't match the defined code!");

	value_type value;
	dwarf_formref(attr, &value, nullptr);
	return value;
}

template <>
Attribute<DW_AT_name>::value_type Attribute<DW_AT_name>::value(const Dwarf_Attribute &attr)
{
	assert(isMatchingType(attr) && "Dwarf_Attribute code doesn't match the defined code!");

	value_type value;
	dwarf_formstring(attr, &value, nullptr);
	return value;
}

template <>
Attribute<DW_AT_encoding>::value_type Attribute<DW_AT_encoding>::value(const Dwarf_Attribute &attr)
{
	assert(isMatchingType(attr) && "Dwarf_Attribute code doesn't match the defined code!");

	value_type value;
	dwarf_formudata(attr, &value, nullptr);
	return value;
}

template <>
Attribute<DW_AT_comp_dir>::value_type Attribute<DW_AT_comp_dir>::value(const Dwarf_Attribute &attr)
{
	assert(isMatchingType(attr) && "Dwarf_Attribute code doesn't match the defined code!");

	value_type value;
	dwarf_formstring(attr, &value, nullptr);
	return value;
}

template <>
Attribute<DW_AT_frame_base>::value_type Attribute<DW_AT_frame_base>::value(const Dwarf_Attribute &attr)
{
	assert(isMatchingType(attr) && "Dwarf_Attribute code doesn't match the defined code!");

	value_type value;
	dwarf_formexprloc(attr, &(value.length), &(value.ptr), nullptr);
	return value;
}

template <>
Attribute<DW_AT_location>::value_type Attribute<DW_AT_location>::value(const Dwarf_Attribute &attr)
{
	assert(isMatchingType(attr) && "Dwarf_Attribute code doesn't match the defined code!");

	value_type value;
	dwarf_formexprloc(attr, &(value.length), &(value.ptr), nullptr);
	return value;
}

template <>
Attribute<DW_AT_upper_bound>::value_type Attribute<DW_AT_upper_bound>::value(const Dwarf_Attribute &attr)
{
	assert(isMatchingType(attr) && "Dwarf_Attribute code doesn't match the defined code!");

	value_type value;
	dwarf_formudata(attr, &value, nullptr);
	return value;
}

template <>
Attribute<DW_AT_byte_size>::value_type Attribute<DW_AT_byte_size>::value(const Dwarf_Attribute &attr)
{
	assert(isMatchingType(attr) && "Dwarf_Attribute code doesn't match the defined code!");

	value_type value;
	dwarf_formudata(attr, &value, nullptr);
	return value;
}

template <>
Attribute<DW_AT_data_member_location>::value_type Attribute<DW_AT_data_member_location>::value(const Dwarf_Attribute &attr)
{
	assert(isMatchingType(attr) && "Dwarf_Attribute code doesn't match the defined code!");

	value_type value;
	dwarf_formudata(attr, &value, nullptr);
	return value;
}

template <>
Attribute<DW_AT_low_pc>::value_type Attribute<DW_AT_low_pc>::value(const Dwarf_Attribute &attr)
{
	assert(isMatchingType(attr) && "Dwarf_Attribute code doesn't match the defined code!");

	value_type value;
	dwarf_formaddr(attr, &value, nullptr);
	return value;
}

template <>
Attribute<DW_AT_high_pc>::value_type Attribute<DW_AT_high_pc>::value(const Dwarf_Attribute &attr)
{
	assert(isMatchingType(attr) && "Dwarf_Attribute code doesn't match the defined code!");

	value_type value;
	dwarf_formudata(attr, &value, nullptr);
	return value;
}

template <>
Attribute<DW_AT_decl_line>::value_type Attribute<DW_AT_decl_line>::value(const Dwarf_Attribute &attr)
{
	assert(isMatchingType(attr) && "Dwarf_Attribute code doesn't match the defined code!");

	value_type value;
	dwarf_formudata(attr, &value, nullptr);
	return value;
}