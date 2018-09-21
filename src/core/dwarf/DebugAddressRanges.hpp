#pragma once

#include <stdint.h>
#include <vector>

#include <libdwarf/libdwarf.h>
#include <libdwarf/dwarf.h>

struct AddressRange
{
	AddressRange(uint64_t start, uint64_t end, uint64_t length) :
		start(start),
		end(end),
		length(length) {}

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
	DebugAddressRanges(const Dwarf_Debug &dbg);

	std::vector<AddressRange> getAddressRanges();

private:
	std::vector<AddressRange> address_ranges;
};