#include "DebuggingInformationEntry.hpp"

#include <dwarf.h>
#include "libdwarf.h"

#include <string>

class DIEBaseType : public DebuggingInformationEntry
{
public:
	DIEBaseType(const Dwarf_Debug& dbg, const Dwarf_Die &die) :
		DebuggingInformationEntry(dbg, die)
	{

	}

	const char *getEncodingMeaning();

private:
	std::string name;
	unsigned int encoding;
	unsigned int byte_size;

	void onAttributeLoaded(const Dwarf_Attribute& attr,
	                       const Dwarf_Half &attr_code,
	                       const Dwarf_Half &form) override;
};