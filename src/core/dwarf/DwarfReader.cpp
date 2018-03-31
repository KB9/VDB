#include "DwarfReader.hpp"

#include <cassert>
#include <algorithm>

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

// =============================================================================
// Attribute
// =============================================================================

Attribute::Attribute(const Dwarf_Attribute &attr) :
	attr(attr)
{
	Dwarf_Error err;

	// Determine the type of the attribute
	Dwarf_Half code;
	if (dwarf_whatattr(attr, &code, &err) != DW_DLV_OK)
		procmsg("[DWARF_ERROR] Error in dwarf_whatattr!\n");
	this->code = code;

	// Determine the form of the attribute
	Dwarf_Half form;
	if (dwarf_whatform(attr, &form, &err) != DW_DLV_OK)
		procmsg("[DWARF_ERROR] Error in dwarf_whatform!\n");
	this->form = form;

	switch (code)
	{
		case DW_AT_type:
		{
			dwarf_formref(attr, &(value.offset), 0);
			procmsg("[DWARF] [Attribute] Saving attribute: DW_AT_type (0x%llx)\n", value.offset);
			value_type = OFFSET;
			break;
		}

		case DW_AT_name:
		{
			dwarf_formstring(attr, &(value.str), 0);
			procmsg("[DWARF] [Attribute] Saving attribute: DW_AT_name (%s)\n", value.str);
			value_type = STRING;
			break;
		}

		case DW_AT_encoding:
		{
			dwarf_formudata(attr, &(value.u_data), 0);
			procmsg("[DWARF] [Attribute] Saving attribute: DW_AT_encoding (%d)\n", value.u_data);
			value_type = UNSIGNED;
			break;
		}

		case DW_AT_byte_size:
		{
			dwarf_formudata(attr, &(value.u_data), 0);
			procmsg("[DWARF] [Attribute] Saving attribute: DW_AT_byte_size (%d)\n", value.u_data);
			value_type = UNSIGNED;
			break;
		}

		case DW_AT_decl_line:
		{
			dwarf_formudata(attr, &(value.u_data), 0);
			procmsg("[DWARF] [Attribute] Saving attribute: DW_AT_decl_line (%d)\n", value.u_data);
			value_type = UNSIGNED;
			break;
		}

		case DW_AT_comp_dir:
		{
			dwarf_formstring(attr, &(value.str), 0);
			procmsg("[DWARF] [Attribute] Save attribute: DW_AT_comp_dir (%s)\n", value.str);
			value_type = STRING;
			break;
		}

		case DW_AT_low_pc:
		{
			dwarf_formaddr(attr, &(value.address), 0);
			procmsg("[DWARF] [Attribute] Saving attribute: DW_AT_low_pc (0x%08x)\n", value.address);
			value_type = ADDRESS;
			break;
		}

		case DW_AT_high_pc:
		{
			switch (form)
			{
				// DWARF 2 implementation
				case DW_FORM_addr:
				{
					dwarf_formaddr(attr, &(value.address), 0);
					procmsg("[DWARF] [Attribute] Saving attribute: DW_AT_high_pc (0x%08x)\n", value.address);
					value_type = ADDRESS;
					break;
				}

				// DWARF 1 implementation
				case DW_FORM_data8:
				{
					dwarf_formudata(attr, &(value.offset), 0);
					procmsg("[DWARF] [Attribute] Saving attribute: DW_AT_high_pc (0x%08x)\n", value.offset);
					value_type = OFFSET;
					break;
				}
			}
			break;
		}

		case DW_AT_location:
		{
			dwarf_formexprloc(attr, &(value.expr_loc.length), &(value.expr_loc.ptr), nullptr);
			procmsg("[DWARF] [Attribute] Saving attribute: DW_AT_location (%d byte block)\n", value.expr_loc.length);
			value_type = EXPRLOC;
			break;
		}

		case DW_AT_data_member_location:
		{
			dwarf_formudata(attr, &(value.u_data), 0);
			procmsg("[DWARF] [Attribute] Saving attribute: DW_AT_data_member_location (%llu)\n", value.u_data);
			value_type = UNSIGNED;
			break;
		}

		case DW_AT_frame_base:
		{
			dwarf_formexprloc(attr, &(value.expr_loc.length), &(value.expr_loc.ptr), nullptr);

			uint8_t *data = (uint8_t *)value.expr_loc.ptr;
			procmsg("[DWARF] [Attribute] Saving attribute: DW_AT_frame_base (0x%x: %d byte block)\n", data[0], value.expr_loc.length);
			value_type = EXPRLOC;
			break;
		}

		case DW_AT_upper_bound:
		{
			dwarf_formudata(attr, &(value.u_data), 0);
			procmsg("[DWARF] [Attribute] Saving attribute: DW_AT_upper_bound (%d)\n", value.u_data);
			value_type = UNSIGNED;
			break;
		}

		default:
		{
			procmsg("[DWARF] [Attribute] Ignoring attribute...\n");
			value_type = IGNORED;
		}
	}
}

Dwarf_Half Attribute::getForm() const
{
	return form;
}

Dwarf_Half Attribute::getCode() const
{
	return code;
}

Dwarf_Off Attribute::getOffset() const
{
	assert(value_type == OFFSET);
	return value.offset;
}

Dwarf_Addr Attribute::getAddress() const
{
	assert(value_type == ADDRESS);
	return value.address;
}

Dwarf_Block *Attribute::getBlock() const
{
	assert(value_type == BLOCK);
	return value.block;
}

std::string Attribute::getString() const
{
	assert(value_type == STRING);
	return value.str;
}

Dwarf_Unsigned Attribute::getUnsigned() const
{
	assert(value_type == UNSIGNED);
	return value.u_data;
}

Dwarf_Ptr Attribute::getPtr() const
{
	assert(value_type == PTR);
	return value.ptr;
}

ExprLoc Attribute::getExprLoc() const
{
	assert(value_type == EXPRLOC);
	return value.expr_loc;
}

// =============================================================================
// DIE
// =============================================================================

DIE::DIE(const Dwarf_Debug &dbg, const Dwarf_Die &die)
{
	this->dbg = dbg;
	this->die = die;
	setTagName();
}

std::vector<Attribute> DIE::getAttributes()
{
	std::vector<Attribute> attributes;

	Dwarf_Error err;
	Dwarf_Attribute *attrs;
	Dwarf_Signed attr_count;

	// Get the list of attributes and the list size
	if (dwarf_attrlist(die, &attrs, &attr_count, &err) != DW_DLV_OK)
		procmsg("[DWARF_ERROR] Error in dwarf_attrlist!\n");

	// Loop through all attributes
	for (int i = 0; i < attr_count; i++)
	{
		attributes.emplace_back(attrs[i]);
	}

	return attributes;
}

Attribute DIE::getAttributeByCode(Dwarf_Half code)
{
	std::vector<Attribute> attrs = getAttributes();
	for (auto &attr : attrs)
	{
		if (attr.getCode() == code)
			return attr;
	}
	assert(false && "Not a valid attribute");
}

DIE *DIE::getParent()
{
	return parent;
}

std::vector<DIE> DIE::getChildren()
{
	std::vector<DIE> children;

	// Check that this DIE has at least 1 child
	Dwarf_Die child_die;
	Dwarf_Error err;
	int result = dwarf_child(die, &child_die, &err);

	// TODO: Perform more extensive error handling here
	if (result != DW_DLV_OK)
	{
		return children;
	}

	children.emplace_back(dbg, child_die);

	// Go over all children DIEs
	while (1)
	{
		// Get the next DIE sibling along
		result = dwarf_siblingof(dbg, child_die, &child_die, &err);

		// TODO: Perform more extensive error handling here
		if (result != DW_DLV_OK)
		{
			break; // Done
		}

		children.emplace_back(dbg, child_die);
	}

	return children;
}

std::string DIE::getTagName()
{
	return tag_name;
}

Dwarf_Off DIE::getOffset()
{
	Dwarf_Off offset;
	Dwarf_Error err;
	if (dwarf_dieoffset(die, &offset, &err) != DW_DLV_OK)
		procmsg("[DWARF_ERROR] Error in dwarf_dieoffset!\n");
	return offset;
}

void DIE::setTagName()
{
	const char *tag_name = 0;
	Dwarf_Half tag;
	Dwarf_Error err;

	// Sets a pointer to the tag of this DIE
	if (dwarf_tag(die, &tag, &err))
		procmsg("[DWARF_ERROR] Error in dwarf_tag!\n");

	// Gets the tag name from the tag pointer
	if (dwarf_get_TAG_name(tag, &tag_name) != DW_DLV_OK)
		procmsg("[DWARF_ERROR] Error in dwarf_get_TAG_name!\n");

	this->tag_name = tag_name;
}

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

bool DIEMatcher::matches(DIE &die)
{
	auto tag_it = std::find(std::begin(tags), std::end(tags), die.getTagName());
	bool is_tag_match = tag_it != std::end(tags);

	bool is_code_match = false;
	std::vector<Attribute> attrs = die.getAttributes();
	for (const auto &attr : attrs)
	{
		auto code_it = std::find(std::begin(attr_codes), std::end(attr_codes), attr.getCode());
		if (code_it != std::end(attr_codes))
		{
			is_code_match = true;
			break;
		}
	}

	return (tags.empty() || is_tag_match) && (attr_codes.empty() || is_code_match);
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
	std::vector<DIE> children = die.getChildren();
	for (auto &child : children)
	{
		std::vector<DIE> sub_children = child.getChildren();
		children.insert(children.end(), sub_children.begin(), sub_children.end());
	}
	return children;
}

DwarfInfoReader::VariableLocExpr DwarfInfoReader::getVarLocExpr(const std::string &var_name)
{
	DwarfInfoReader::VariableLocExpr loc_expr;

	DIEMatcher matcher;
	matcher.setTags({"DW_TAG_subprogram"});

	std::vector<DIE> subprograms = getDIEs(matcher);
	for (auto &sub : subprograms)
	{
		// Get the frame base for this subprogram
		Attribute frame_base = sub.getAttributeByCode(DW_AT_frame_base);
		loc_expr.frame_base = ((uint8_t *)frame_base.getExprLoc().ptr)[0];

		// Check all of its children to check for a local variable with the
		// name
		std::vector<DIE> children = sub.getChildren();
		for (auto &child : children)
		{
			// Only look at variables and formal parameters
			if (child.getTagName() != "DW_TAG_variable" &&
			    child.getTagName() != "DW_TAG_formal_parameter")
				continue;

			// Ensure that the name of this variable matches before getting the
			// location expression
			Attribute name = child.getAttributeByCode(DW_AT_name);
			if (name.getString() != var_name)
				continue;

			Attribute location = child.getAttributeByCode(DW_AT_location);
			loc_expr.location_op = ((uint8_t *)location.getExprLoc().ptr)[0];
			loc_expr.location_param = &((uint8_t *)location.getExprLoc().ptr)[1];

			Attribute type = child.getAttributeByCode(DW_AT_type);
			loc_expr.type = getDIEByOffset(type.getOffset());
		}
	}

	return loc_expr;
}