#pragma once

#include <memory>
#include <vector>

#include <dwarf.h>
#include "libdwarf.h"

#include <string>

// Developing a giant structure like this with everything pre-loaded into
// variables is a bit of an anti-pattern. Should I change it so that
// everything is accessible via getter methods?
// I think it would make sense, as I'm currently creating it in a way that
// requires everything to be initialized in the constructor, which is making
// my job significantly harder.
//
// Anything that requires a lot of processing time to construct will be made
// available immediately after initialization. Otherwise, getter methods.
//
// Better yet, anything that represents the core nodes of the .debug_info
// tree should be made readily available after initialization. Any leaf
// nodes should be made into getter methods.

class DebuggingInformationEntry
{
public:
	DebuggingInformationEntry(Dwarf_Debug dbg, Dwarf_Die die);

	std::vector<std::unique_ptr<DebuggingInformationEntry>> &getChildren();
	
private:
	void loadChildren(Dwarf_Debug dbg);
	void loadTag(Dwarf_Debug dbg);
	
	// Internal libdwarf type
	Dwarf_Die die;

	// The tag name associated with this DIE
	//std::string tag;

	std::vector<std::unique_ptr<DebuggingInformationEntry>> children;
};

class CUHeader
{
public:
	CUHeader(Dwarf_Debug dbg);

	std::unique_ptr<DebuggingInformationEntry> root_die;
	
	Dwarf_Unsigned length;
	Dwarf_Half version_stamp;
	Dwarf_Unsigned abbrev_offset;
	Dwarf_Half address_size;
	Dwarf_Unsigned next_cu_header;
};

// Represents the information presented when performing a call to objdump --dwarf=info
class DebugInfo
{
public:
	DebugInfo();

	std::vector<std::unique_ptr<CUHeader>> &getCUHeaders();
private:
	void loadCUHeaders(Dwarf_Debug dbg);
	
	std::vector<std::unique_ptr<CUHeader>> headers;
};
