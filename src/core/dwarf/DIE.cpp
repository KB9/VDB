#include "DIE.hpp"

#include <cassert>

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

DIE::DIE(const Dwarf_Debug &dbg, const Dwarf_Die &die)
{
	this->dbg = dbg;
	this->die = die;
	setTagName();
}

Dwarf_Off DIE::getCUOffset()
{
	Dwarf_Off cu_offset;
	Dwarf_Error err;
	if (dwarf_CU_dieoffset_given_die(die, &cu_offset, &err) != DW_DLV_OK)
		procmsg("[DWARF_ERROR] Error in dwarf_die_CU_offset!\n");
	return cu_offset;
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

std::string DIE::getTagName() const
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

const Dwarf_Die &DIE::get() const
{
	return die;
}