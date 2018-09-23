#ifndef _DIE_H_
#define _DIE_H_

#include <vector>
#include <string>
#include <optional>

#include <libdwarf/dwarf.h>
#include <libdwarf/libdwarf.h>

#include "../expected.hpp"
using namespace nonstd;

#include "Attribute.hpp"

class DIE
{
public:
	DIE(const Dwarf_Debug &dbg, const Dwarf_Die &die);

	template <Dwarf_Half CODE>
	expected<typename Attribute<CODE>::value_type, std::string> getAttributeValue() const
	{
		using ValueType = typename Attribute<CODE>::value_type;

		Dwarf_Error err;
		Dwarf_Attribute *attrs;
		Dwarf_Signed attr_count;

		// Get the list of attributes and the list size
		if (dwarf_attrlist(die, &attrs, &attr_count, &err) != DW_DLV_OK)
			return make_unexpected("Failed to read DIE attributes");

		// Loop through all attributes
		for (int i = 0; i < attr_count; i++)
		{
			if (Attribute<CODE>::isMatchingType(attrs[i]))
				return Attribute<CODE>::value(attrs[i]);
		}
		return make_unexpected("Failed to find matching DIE attribute");
	}

	template <Dwarf_Half CODE>
	bool hasAttribute() const
	{
		return getAttributeValue<CODE>().has_value();
	}

	Dwarf_Off getCUOffset();
	std::vector<DIE> getChildren();
	std::string getTagName() const;
	Dwarf_Off getOffset();

	const Dwarf_Die &get() const;

private:
	Dwarf_Debug dbg;
	Dwarf_Die die;
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

#endif // _DIE_H_