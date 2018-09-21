#ifndef _ATTRIBUTE_H_
#define _ATTRIBUTE_H_

#include <string>
#include <cassert>

#include <libdwarf/dwarf.h>
#include <libdwarf/libdwarf.h>

struct ExprLoc
{
	Dwarf_Unsigned length;
	Dwarf_Ptr ptr;
};

// ================ Mapping attribute tags to their value types ================

template <Dwarf_Half CODE>
struct AttributeCode {};

template <>
struct AttributeCode<DW_AT_type>
{
	typedef Dwarf_Off value_type;
};

template <>
struct AttributeCode<DW_AT_name>
{
	typedef char * value_type;
};

template <>
struct AttributeCode<DW_AT_encoding>
{
	typedef Dwarf_Unsigned value_type;
};

template <>
struct AttributeCode<DW_AT_comp_dir>
{
	typedef char * value_type;
};

template <>
struct AttributeCode<DW_AT_frame_base>
{
	typedef ExprLoc value_type;
};

template <>
struct AttributeCode<DW_AT_location>
{
	typedef ExprLoc value_type;
};

template <>
struct AttributeCode<DW_AT_upper_bound>
{
	typedef Dwarf_Unsigned value_type;
};

template <>
struct AttributeCode<DW_AT_byte_size>
{
	typedef Dwarf_Unsigned value_type;
};

template <>
struct AttributeCode<DW_AT_data_member_location>
{
	typedef Dwarf_Unsigned value_type;
};

template <>
struct AttributeCode<DW_AT_low_pc>
{
	typedef Dwarf_Addr value_type;
};

template <>
struct AttributeCode<DW_AT_high_pc>
{
	// TODO: This type can change depending on the DWARF version. Use libdwarf
	// impl to read this value (and low pc value)
	typedef Dwarf_Off value_type;
};

template <>
struct AttributeCode<DW_AT_decl_line>
{
	typedef Dwarf_Unsigned value_type;
};

// ================ Calculating the values for the value types ================

template <Dwarf_Half CODE>
class Attribute
{
public:
	typedef typename AttributeCode<CODE>::value_type value_type;

	static bool isMatchingType(const Dwarf_Attribute &attr);
	static value_type value(const Dwarf_Attribute &attr);
};

#endif // _ATTRIBUTE_H_