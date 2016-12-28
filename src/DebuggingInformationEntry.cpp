#include "DebuggingInformationEntry.hpp"

#include "DIESubprogram.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

// TODO: Should probably be a helper method...
std::string getDieTagName(Dwarf_Die die)
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

	return tag_name;
}

DebuggingInformationEntry::DebuggingInformationEntry(Dwarf_Debug dbg, Dwarf_Die die)
{
	this->die = die;
	
	loadChildren(dbg);
	//loadAttributes();
}

void DebuggingInformationEntry::loadChildren(Dwarf_Debug dbg)
{
	procmsg("Loading children of: %s\n", getTagName(dbg).c_str());

	// Check that this DIE has at least 1 child
	Dwarf_Die child_die;
	Dwarf_Error err;
	int result = dwarf_child(die, &child_die, &err);

	// TODO: Perform more extensive error handling here
	if (result != DW_DLV_OK)
	{
		return;
	}

	// Add the first child if it is a subprogram
	if (getDieTagName(child_die) == "DW_TAG_subprogram")
	{
		children.push_back(std::unique_ptr<DebuggingInformationEntry>(new DIESubprogram(dbg, child_die)));
		children.back()->loadAttributes();
	}


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

		// Add the child if it is a subprogram
		if (getDieTagName(child_die) == "DW_TAG_subprogram")
		{
			children.push_back(std::unique_ptr<DebuggingInformationEntry>(new DIESubprogram(dbg, child_die)));
			children.back()->loadAttributes();
		}
	}
}

void DebuggingInformationEntry::loadAttributes()
{
	Dwarf_Error err;
	Dwarf_Attribute *attrs;
	Dwarf_Signed attr_count;

	// Get the list of attributes and the list size
	if (dwarf_attrlist(die, &attrs, &attr_count, &err) != DW_DLV_OK)
		procmsg("[DWARF_ERROR] Error in dwarf_attrlist!\n");

	// Loop through all attributes
	for (int i = 0; i < attr_count; i++)
	{
		// Determine the type of the attribute
		Dwarf_Half attr_code;
		if (dwarf_whatattr(attrs[i], &attr_code, &err) != DW_DLV_OK)
			procmsg("[DWARF_ERROR] Error in dwarf_whatattr!\n");

		// Notify subclasses that an attribute has been found
		onAttributeLoaded(attrs[i], attr_code);
	}
}

std::string DebuggingInformationEntry::getTagName(Dwarf_Debug dbg)
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

	return tag_name;
}

std::vector<std::unique_ptr<DebuggingInformationEntry>> &DebuggingInformationEntry::getChildren()
{
	return children;
}
