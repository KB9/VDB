#pragma once

#include <stdint.h>
#include <vector>

#include "libdwarf.h"
#include <dwarf.h>

struct AddressRange
{
	const uint64_t start;
	const uint64_t end;
	const uint64_t length;
};

// Lookup table for mapping addresses to compilation units.
// Represents the information presented when performing a call to
// objdump --dwarf=aranges.
class DebugAddressRanges
{
public:
	DebugAddressRanges(Dwarf_Debug dbg);

	std::vector<AddressRange> getAddressRanges();

private:
	std::vector<AddressRange> address_ranges;
};