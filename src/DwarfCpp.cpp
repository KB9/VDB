#include "DwarfCpp.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
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
