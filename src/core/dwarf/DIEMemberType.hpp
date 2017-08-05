#pragma once

#include "DebuggingInformationEntry.hpp"

class DIEMemberType : public DebuggingInformationEntry
{
public:
	DIEMemberType(const Dwarf_Debug &dbg,
	              const Dwarf_Die &die,
	              DebuggingInformationEntry *parent) :
		DebuggingInformationEntry(dbg, die, parent)
	{

	}

	std::string getName() const;
	uint64_t getDeclLineNumber() const;
	uint64_t getTypeOffset() const;
	uint64_t getDataMemberLocation() const;

private:
	std::string name;
	uint64_t decl_line_number;
	uint64_t type_offset;
	uint64_t data_member_location;

	virtual void onAttributeLoaded(const Dwarf_Attribute &attr,
	                               const Dwarf_Half &attr_code,
	                               const Dwarf_Half &form) override;
};