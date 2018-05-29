#include "DwarfDebug.hpp"

#include <cstring>

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

static void simple_error_handler(Dwarf_Error error, Dwarf_Ptr errarg)
{
	printf("libdwarf error: %llu %s0", dwarf_errno(error), dwarf_errmsg(error));
}

DwarfDebug::DwarfDebug(std::string filename)
{
	// Open the file in question
	file = fopen(filename.c_str(), "r");
	int file_descriptor = fileno(file);

	// Initialize libdwarf
	Dwarf_Ptr errarg;
	dwarf_init(file_descriptor, DW_DLC_READ, simple_error_handler, &errarg, &dbg, NULL);

	// Initialize the various DWARF debugging components
	debug_info = std::make_shared<DwarfInfoReader>(dbg);
	debug_line = std::make_shared<DebugLine>(debug_info->getCompileUnits());
	debug_aranges = std::make_shared<DebugAddressRanges>(dbg);
}

DwarfDebug::~DwarfDebug()
{
	// Finish using libdwarf and close the file stream
	int result = dwarf_finish(dbg, NULL);
	if (result != DW_DLV_OK)
	{
		procmsg("[DWARF_ERROR] Error during finishing operation!\n");
	}
	fclose(file);
}

std::shared_ptr<DwarfInfoReader> DwarfDebug::info()
{
	return debug_info;
}

std::shared_ptr<DebugLine> DwarfDebug::line()
{
	return debug_line;
}

std::shared_ptr<DebugAddressRanges> DwarfDebug::aranges()
{
	return debug_aranges;
}

std::vector<SourceFile> sourceFiles(std::shared_ptr<DwarfDebug> debug_data)
{
	std::vector<SourceFile> files;

	std::vector<DIE> compile_units = debug_data->info()->getCompileUnits();
	for (auto &cu : compile_units)
	{
		SourceFile file;
		char default_name[] = "<file_name_not_found>";
		char default_dir[] = "<file_dir_not_found>";
		file.name = cu.getAttributeValue<DW_AT_name>().value_or(default_name);
		file.dir = cu.getAttributeValue<DW_AT_comp_dir>().value_or(default_dir);
		files.push_back(file);
	}
	return files;
}