// #pragma once

// #include <vector>
// #include <memory>
// #include <unordered_map>

// #include "libdwarf.h"
// #include <dwarf.h>

// #include "CUHeader.hpp"
// #include "DebuggingInformationEntry.hpp"

// using OffsetMap =
// 	std::unordered_map<unsigned long long, std::shared_ptr<DebuggingInformationEntry>>;

// // Represents the information presented when performing a call to objdump --dwarf=info
// class DebugInfo
// {
// public:
// 	DebugInfo(const Dwarf_Debug &dbg);

// 	std::vector<CUHeader> &getCUHeaders();

// 	std::shared_ptr<DebuggingInformationEntry> getDIEByOffset(unsigned long long offset);
// private:
// 	void loadCUHeaders();

// 	Dwarf_Debug dbg;
	
// 	std::vector<CUHeader> headers;

// 	OffsetMap dies_by_offset;
// 	void loadOffsetMap(std::shared_ptr<DebuggingInformationEntry> die, OffsetMap &offset_map);
// };