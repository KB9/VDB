#pragma once

/*
TODO:

I need to create a lazily loaded DIE structure for this compilation unit. Time
for some brainstorming:

1)
AIM:
This could be a tree like I had before but, instead of instantiating all the
children of the compile unit DIE, I could only instantiate the ones that are
required. This would be akin to a guided DFS where only branches that are used
are instantiated and memoised.
PROBLEMS:
How would I direct the instantiation path?: The subprogram DIEs have an address
range. If I know the address for my search, I can consecutively narrow the
address range by selecting subprograms that contain the address within their
address ranges.
The above doesn't work... Subprograms can be located anywhere in memory.
*/

#include <tuple>
#include <vector>
#include <string>
#include <memory>

#include <dwarf.h>
#include "libdwarf.h"

struct ExprLoc
{
	Dwarf_Unsigned length;
	Dwarf_Ptr ptr;
};

union AttributeValue
{
	Dwarf_Off offset;
	Dwarf_Addr address;
	Dwarf_Block *block;
	char *str;
	Dwarf_Unsigned u_data;
	Dwarf_Ptr ptr;
	ExprLoc expr_loc;
};

enum AttributeType
{
	IGNORED,
	OFFSET,
	ADDRESS,
	BLOCK,
	STRING,
	UNSIGNED,
	PTR,
	EXPRLOC
};

// NOTE: This is nearly exactly how GDB handles attributes, except that it is
// a struct instead of a class.
// TODO: These attributes need their names as well.
class Attribute
{
public:
	Attribute(const Dwarf_Attribute &attr);

	Dwarf_Half getForm() const;
	Dwarf_Half getCode() const;

	Dwarf_Off getOffset() const;
	Dwarf_Addr getAddress() const;
	Dwarf_Block *getBlock() const;
	std::string getString() const;
	Dwarf_Unsigned getUnsigned() const;
	Dwarf_Ptr getPtr() const;
	ExprLoc getExprLoc() const;

private:
	Dwarf_Attribute attr;
	Dwarf_Half form;
	Dwarf_Half code;

	AttributeValue value;
	AttributeType value_type;
};

class DIE
{
public:
	DIE(const Dwarf_Debug &dbg, const Dwarf_Die &die);

	std::vector<Attribute> getAttributes() const;
	Attribute getAttributeByCode(Dwarf_Half code) const;
	DIE *getParent();
	std::vector<DIE> getChildren();
	std::string getTagName() const;
	Dwarf_Off getOffset();

	Dwarf_Die die; // NOTE: TEMPORARILY PUBLIC FOR TESTING/DEBUG PURPOSES

private:
	Dwarf_Debug dbg;
	// Dwarf_Die die;
	DIE *parent = nullptr;
	std::string tag_name;

	void setTagName();
};

class DIEMatcher
{
public:
	DIEMatcher &setTags(const std::vector<std::string> &tags);
	DIEMatcher &setAttrCodes(const std::vector<Dwarf_Half> &attr_codes);

	bool matches(DIE &die);

private:
	std::vector<std::string> tags;
	std::vector<Dwarf_Half> attr_codes;
};

// This will contain the compilation units, and allow access to each of them.
// A compilation unit has a specific address range, and I could perhaps narrow
// the search for a variable by looking at the high and low PC values for each
// compilation unit.
class DwarfInfoReader
{
public:
	DwarfInfoReader(const Dwarf_Debug &dbg);

	std::vector<DIE> getCompileUnits();

	std::unique_ptr<DIE> getDIEByOffset(Dwarf_Off offset);

	std::vector<DIE> getDIEs(DIEMatcher &matcher);
	std::vector<DIE> getChildrenRecursive(DIE &die);

	struct VariableLocExpr
	{
		uint8_t frame_base;
		uint8_t location_op;
		uint8_t *location_param;
		std::unique_ptr<DIE> type;
	};
	VariableLocExpr getVarLocExpr(const std::string &var_name);

private:
	Dwarf_Debug dbg;
};