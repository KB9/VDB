#include "DwarfCpp.hpp"

// FOWARD DECLARATION
void procmsg(const char* format, ...);

static void simple_error_handler(Dwarf_Error error, Dwarf_Ptr errarg)
{
	printf("libdwarf error: %d %s0", dwarf_errno(error), dwarf_errmsg(error));
}

// TODO: Find a structure to put this in
FILE *file = nullptr;

Dwarf_Debug init()
{
	// Open the file in question (HORRIBLE HARDCODING)
	file = fopen("../../../testchild", "r");
	int file_descriptor = fileno(file);

	// Initialize libdwarf and Dwarf_Debug
	Dwarf_Ptr errarg;
	Dwarf_Debug dbg;
	dwarf_init(file_descriptor, DW_DLC_READ, simple_error_handler, &errarg, &dbg, NULL);

	return dbg;
}

void finish(Dwarf_Debug dbg)
{
	// Finish using libdwarf and close the file stream
	int result = dwarf_finish(dbg, NULL);
	if (result != DW_DLV_OK)
	{
		procmsg("[DWARF_ERROR] Error during finishing operation!\n");
	}
	fclose(file);
}





DebugInfo::DebugInfo()
{
	Dwarf_Debug dbg = init();
	
	// Load the compilation unit headers
	loadCUHeaders(dbg);
}

void DebugInfo::loadCUHeaders(Dwarf_Debug dbg)
{
	Dwarf_Error err;
	int result = DW_DLV_OK;
	while (result == DW_DLV_OK)
	{
		Dwarf_Unsigned cu_header_length, abbrev_offset, next_cu_header;
		Dwarf_Half version_stamp, address_size;
		Dwarf_Error err;
		result = dwarf_next_cu_header(
	            dbg,
		        &cu_header_length,
		        &version_stamp,
		        &abbrev_offset,
		        &address_size,
		        &next_cu_header,
		        &err);

		if (result == DW_DLV_OK)
		{
	        std::unique_ptr<CUHeader> header(new CUHeader(dbg));
			header->length = cu_header_length;
			header->version_stamp = version_stamp;
			header->abbrev_offset = abbrev_offset;
			header->address_size = address_size;
			header->next_cu_header = next_cu_header;

			headers.push_back(std::move(header));
        }
	}

	// Check in case of error loading
	if (result == DW_DLV_ERROR)
		procmsg("[DWARF_ERROR] Error loading CU headers!\n");
}

std::vector<std::unique_ptr<CUHeader>> &DebugInfo::getCUHeaders()
{
	return headers;
}





CUHeader::CUHeader(Dwarf_Debug dbg)
{
	Dwarf_Die no_die = 0, cu_die;
	Dwarf_Error err;

	// Expect the CU to have a single sibling - a DIE.
	if (dwarf_siblingof(dbg, no_die, &cu_die, &err) == DW_DLV_ERROR)
		procmsg("[DWARF_ERROR] Error getting sibling of CU! %s\n", dwarf_errmsg(err));

	// Initialize the root DIE and assign its internal type
	root_die = std::make_unique<DebuggingInformationEntry>(dbg, cu_die);
}





DebuggingInformationEntry::DebuggingInformationEntry(Dwarf_Debug dbg, Dwarf_Die die)
{
	this->die = die;

	loadTag(dbg);
	
	loadChildren(dbg);
}

void DebuggingInformationEntry::loadChildren(Dwarf_Debug dbg)
{
	// Check that this DIE has at least 1 child
	Dwarf_Die child_die;
	Dwarf_Error err;
	int result = dwarf_child(die, &child_die, &err);

	// TODO: Perform more extensive error handling here
	if (result != DW_DLV_OK)
	{
		return;
	}

	// Add the confirmed 1 child to the list of children
	children.push_back(std::make_unique<DebuggingInformationEntry>(dbg, child_die));

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

		// Add the child's sibling DIE to the list of children
		children.push_back(std::make_unique<DebuggingInformationEntry>(dbg, child_die));
	}
}

void DebuggingInformationEntry::loadTag(Dwarf_Debug dbg)
{
	const char *tag_name = 0;
	Dwarf_Half tag;
	Dwarf_Error err;
	
	if (dwarf_tag(die, &tag, &err))
		procmsg("[DWARF_ERROR] Error in dwarf_tag!\n");

	if (dwarf_get_TAG_name(tag, &tag_name) != DW_DLV_OK)
		procmsg("[DWARF_ERROR] Error in dwarf_get_TAG_name!\n");

	procmsg("[DWARF] DIE tag name: %s\n", tag_name);
}

std::vector<std::unique_ptr<DebuggingInformationEntry>> &DebuggingInformationEntry::getChildren()
{
	return children;
}
