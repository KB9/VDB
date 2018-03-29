// #include "DebugInfo.hpp"

// // FOWARD DECLARATION [TODO: REMOVE]
// void procmsg(const char* format, ...);

// DebugInfo::DebugInfo(const Dwarf_Debug &dbg) : dbg(dbg)
// {
// 	// Load the compilation unit headers
// 	loadCUHeaders();

// 	// Load the offsets of each DIE into the offset map
// 	for (auto cu_header : getCUHeaders())
// 	{
// 		loadOffsetMap(cu_header.root_die, dies_by_offset);
// 	}
// }

// void DebugInfo::loadCUHeaders()
// {
// 	int result = DW_DLV_OK;
// 	while (result == DW_DLV_OK)
// 	{
// 		Dwarf_Unsigned cu_header_length, abbrev_offset, next_cu_header;
// 		Dwarf_Half version_stamp, address_size;
// 		Dwarf_Error err;
// 		result = dwarf_next_cu_header(
// 	            dbg,
// 		        &cu_header_length,
// 		        &version_stamp,
// 		        &abbrev_offset,
// 		        &address_size,
// 		        &next_cu_header,
// 		        &err);

// 		if (result == DW_DLV_OK)
// 		{
// 			CUHeader header(dbg);
// 			header.length = cu_header_length;
// 			header.version_stamp = version_stamp;
// 			header.abbrev_offset = abbrev_offset;
// 			header.address_size = address_size;
// 			header.next_cu_header = next_cu_header;

// 			headers.push_back(std::move(header));
//         }
// 	}

// 	// Check in case of error loading
// 	if (result == DW_DLV_ERROR)
// 		procmsg("[DWARF_ERROR] Error loading CU headers!\n");
// }

// std::vector<CUHeader> &DebugInfo::getCUHeaders()
// {
// 	return headers;
// }

// void DebugInfo::loadOffsetMap(std::shared_ptr<DebuggingInformationEntry> die,
//                               OffsetMap &offset_map)
// {
// 	Dwarf_Off offset;
// 	dwarf_dieoffset(die->getInternalDie(), &offset, 0);
// 	offset_map[offset] = die;
// 	procmsg("[DIE_OFFSET] Offset 0x%x -> Address 0x%x\n", offset, &(*die));

// 	for (auto &child_die : die->getChildren())
// 	{
// 		loadOffsetMap(child_die, offset_map);
// 	}
// }

// // TODO: Need to find out how to directly query DWARF data by offset (in order
// // to replace this method which is being removed)!
// std::shared_ptr<DebuggingInformationEntry> DebugInfo::getDIEByOffset(unsigned long long offset)
// {
// 	for (auto kv_pair : dies_by_offset)
// 	{
// 		if (kv_pair.first == offset)
// 		{
// 			return kv_pair.second;
// 		}
// 	}
// 	return nullptr;
// }