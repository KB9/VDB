#pragma once

#include "DebuggingInformationEntry.hpp"

#include <string>

class DIEStructureType : public DebuggingInformationEntry
{
public:
	DIEStructureType(const Dwarf_Debug &dbg,
	                 const Dwarf_Die &die,
	                 DebuggingInformationEntry *parent) :
		DebuggingInformationEntry(dbg, die, parent)
	{

	}

	std::string getName() const;
	uint64_t getDeclLineNumber() const;
	uint64_t getByteSize() const;

private:
	std::string name;
	uint64_t decl_line_number;
	uint64_t byte_size;

	virtual void onAttributeLoaded(const Dwarf_Attribute &attr,
	                               const Dwarf_Half &attr_code,
	                               const Dwarf_Half &form) override;
};