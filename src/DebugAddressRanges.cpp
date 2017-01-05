#include "DebugAddressRanges.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

DebugAddressRanges::DebugAddressRanges(Dwarf_Debug dbg)
{
	Dwarf_Arange *aranges;
	Dwarf_Signed arange_count;
	Dwarf_Error err;

	// Get all address ranges
	int result = dwarf_get_aranges(dbg, &aranges, &arange_count, &err);
	if (result == DW_DLV_NO_ENTRY)
	{
		procmsg("[DWARF] .debug_aranges does not exist!\n");
		return;
	}
	else if (result == DW_DLV_ERROR)
	{
		procmsg("[DWARF_ERROR] Error in dwarf_get_aranges!\n");
		return;
	}

	// Loop through all aranges
	for (int i = 0; i < arange_count; i++)
	{
		Dwarf_Addr start;
		Dwarf_Unsigned length;
		Dwarf_Off cu_die_offset;

		// Get information about the address range
		result = dwarf_get_arange_info(
				aranges[i],
				&start,
				&length,
				&cu_die_offset,
				&err);
		if (result == DW_DLV_ERROR)
		{
			procmsg("[DWARF_ERROR] Error in dwarf_get_arange_info!\n");
			return;
		}

		// Create the address range and push it to the vector
		AddressRange range = {
			.start = start,
			.end = start + length,
			.length = length
		};
		address_ranges.push_back(range);

		// DEBUG
		procmsg("[DWARF] ========== ADDRESS RANGE ==========\n");
		procmsg("[DWARF] Start: %lx End: %lx Length: %lx\n", start, start + length, length);
	}
}

std::vector<AddressRange> DebugAddressRanges::getAddressRanges()
{
	return address_ranges;
}