#include "DwarfReader.hpp"

#include <cassert>
#include <algorithm>

#include <sys/ptrace.h>

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

// =============================================================================
// DIEMatcher
// =============================================================================

DIEMatcher &DIEMatcher::setTags(const std::vector<std::string> &tags)
{
	this->tags = tags;
	return *this;
}

DIEMatcher &DIEMatcher::setAttrCodes(const std::vector<Dwarf_Half> &attr_codes)
{
	this->attr_codes = attr_codes;
	return *this;
}

// bool DIEMatcher::matches(DIE &die)
// {
// 	auto tag_it = std::find(std::begin(tags), std::end(tags), die.getTagName());
// 	bool is_tag_match = tag_it != std::end(tags);

// 	bool is_code_match = false;
// 	std::vector<Attribute> attrs = die.getAttributes();
// 	for (const auto &attr : attrs)
// 	{
// 		auto code_it = std::find(std::begin(attr_codes), std::end(attr_codes), attr.getCode());
// 		if (code_it != std::end(attr_codes))
// 		{
// 			is_code_match = true;
// 			break;
// 		}
// 	}

// 	return (tags.empty() || is_tag_match) && (attr_codes.empty() || is_code_match);
// }
bool DIEMatcher::matches(DIE &die)
{
	auto tag_it = std::find(std::begin(tags), std::end(tags), die.getTagName());
	bool is_tag_match = tag_it != std::end(tags);

	// bool is_code_match = true;
	// for (const auto &code : attr_codes)
	// {
	// 	is_code_match &= die.hasAttribute<code>();
	// }
	return (tags.empty() || is_tag_match);// && (attr_codes.empty() || is_code_match);
}

// =============================================================================
// DwarfInfoReader
// =============================================================================

DwarfInfoReader::DwarfInfoReader(const Dwarf_Debug &dbg)
{
	this->dbg = dbg;
}

std::vector<DIE> DwarfInfoReader::getCompileUnits()
{
	std::vector<DIE> compile_units;

	// Iterate over all compilation unit headers until the end is reached
	Dwarf_Unsigned cu_header_length, abbrev_offset, next_cu_header;
	Dwarf_Half version_stamp, address_size;
	Dwarf_Error err;
	int result = dwarf_next_cu_header(dbg, &cu_header_length, &version_stamp,
	                                  &abbrev_offset, &address_size,
	                                  &next_cu_header, &err);
	while (result == DW_DLV_OK)
	{
		// Find the compilation unit associated with this header
		Dwarf_Die no_die = 0, current_die;
		result = dwarf_siblingof(dbg, no_die, &current_die, &err);
		if (result == DW_DLV_OK)
		{
			// If the compilation unit was found, add it to the list
			auto cu = std::make_unique<DIE>(dbg, current_die);
			compile_units.push_back(*cu);

			result = dwarf_next_cu_header(dbg, &cu_header_length, &version_stamp,
			                              &abbrev_offset, &address_size,
			                              &next_cu_header, &err);
		}
	}

	return compile_units;
}

std::unique_ptr<DIE> DwarfInfoReader::getDIEByOffset(Dwarf_Off offset)
{
	Dwarf_Die found_die;
	Dwarf_Error err;
	int result = dwarf_offdie(dbg, offset, &found_die, &err);
	if (result == DW_DLV_OK)
	{
		return std::make_unique<DIE>(dbg, found_die);
	}
	else
	{
		return nullptr;
	}
}

std::vector<DIE> DwarfInfoReader::getDIEs(DIEMatcher &matcher)
{
	std::vector<DIE> results;

	std::vector<DIE> compile_units = getCompileUnits();
	for (auto &cu : compile_units)
	{
		std::vector<DIE> all_children = getChildrenRecursive(cu);
		for (auto &child : all_children)
		{
			if (matcher.matches(child))
				results.push_back(child);
		}
	}
	return results;
}

std::vector<DIE> DwarfInfoReader::getChildrenRecursive(DIE &die)
{
	std::vector<DIE> all_children;

	std::vector<DIE> children = die.getChildren();
	all_children.insert(all_children.end(), children.begin(), children.end());
	for (auto &child : children)
	{
		std::vector<DIE> sub_children = getChildrenRecursive(child);
		all_children.insert(all_children.end(), sub_children.begin(), sub_children.end());
	}
	return all_children;
}

expected<DwarfInfoReader::VariableLocExpr, std::string> DwarfInfoReader::getVarLocExpr(const std::string &var_name,
                                                                                       pid_t pid)
{
	DwarfInfoReader::VariableLocExpr loc_expr;

	DIEMatcher matcher;
	matcher.setTags({"DW_TAG_subprogram"});

	std::vector<DIE> subprograms = getDIEs(matcher);
	for (auto &sub : subprograms)
	{
		// Ensure that the variable is within the address range of the function
		// the IP is currently within
		auto expected_low_pc = sub.getAttributeValue<DW_AT_low_pc>();
		auto expected_high_pc = sub.getAttributeValue<DW_AT_high_pc>();
		if (!expected_low_pc || !expected_high_pc)
			continue;

		user_regs_struct regs;
		ptrace(PTRACE_GETREGS, pid, 0, &regs);
		uint64_t low_pc = expected_low_pc.value();
		uint64_t high_pc = expected_high_pc.value();
		if (regs.rip < low_pc || regs.rip >= (low_pc + high_pc))
			continue;

		// Determine if this subprogram DIE has a frame base attribute
		auto expected_frame_base = sub.getAttributeValue<DW_AT_frame_base>();
		if (!expected_frame_base)
			continue;

		// Cast the frame base exprloc and store
		loc_expr.frame_base = ((uint8_t *)expected_frame_base.value().ptr)[0];

		// Check all of this subprogram's children to check for a local variable
		// with the same name
		std::vector<DIE> children = sub.getChildren();
		for (auto &child : children)
		{
			// Only look at variables and formal parameters
			if (child.getTagName() != "DW_TAG_variable" &&
			    child.getTagName() != "DW_TAG_formal_parameter")
			    continue;

			// Ensure that the name of this variable matches before getting the
			// location expression
			std::string name = child.getAttributeValue<DW_AT_name>().value();
			if (name != var_name)
				continue;

			// Ensure the variable has a location expression attribute
			auto expected_loc = child.getAttributeValue<DW_AT_location>();
			if (!expected_loc)
				continue;

			// Cast the location op and param and store
			loc_expr.location_op = ((uint8_t *)expected_loc.value().ptr)[0];
			loc_expr.location_param = &((uint8_t *)expected_loc.value().ptr)[1];

			// Determine the DIE that represents the type using type offset attribute
			Dwarf_Off type_offset = child.getAttributeValue<DW_AT_type>().value();
			loc_expr.type = getDIEByOffset(type_offset);

			return loc_expr;
		}
	}

	// Then look for the variable globally if it isn't found locally
	loc_expr.frame_base = 0;
	std::vector<DIE> compile_units = getCompileUnits();
	for (auto &cu : compile_units)
	{
		std::vector<DIE> cu_children = cu.getChildren();
		for (auto &child : cu_children)
		{
			if (child.getTagName() != "DW_TAG_variable")
				continue;

			// Ensure that the name of this variable matches before getting the
			// location expression
			std::string name = child.getAttributeValue<DW_AT_name>().value();
			if (name != var_name)
				continue;

			// Ensure the variable has a location expression attribute
			auto expected_loc = child.getAttributeValue<DW_AT_location>();
			if (!expected_loc)
				continue;

			// Cast the location op and param and store
			loc_expr.location_op = ((uint8_t *)expected_loc.value().ptr)[0];
			loc_expr.location_param = &((uint8_t *)expected_loc.value().ptr)[1];

			// Determine the DIE that represents the type using type offset attribute
			Dwarf_Off type_offset = child.getAttributeValue<DW_AT_type>().value();
			loc_expr.type = getDIEByOffset(type_offset);

			return std::move(loc_expr);
		}
	}

	return make_unexpected("Could not determine location expression: " + var_name);
}