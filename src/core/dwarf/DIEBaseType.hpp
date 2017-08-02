#pragma once

#include "DebuggingInformationEntry.hpp"

#include <dwarf.h>
#include "libdwarf.h"

#include <string>

class DIEBaseType : public DebuggingInformationEntry
{
public:
	DIEBaseType(const Dwarf_Debug& dbg,
	            const Dwarf_Die &die,
	            DebuggingInformationEntry *parent) :
		DebuggingInformationEntry(dbg, die, parent)
	{

	}

	unsigned int getEncoding() const;
	const char *getEncodingMeaning();

	unsigned int getByteSize() const;

private:
	std::string name;
	unsigned int encoding;
	unsigned int byte_size;

	void onAttributeLoaded(const Dwarf_Attribute& attr,
	                       const Dwarf_Half &attr_code,
	                       const Dwarf_Half &form) override;
};