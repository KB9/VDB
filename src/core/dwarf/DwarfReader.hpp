#pragma once

#include <tuple>
#include <vector>
#include <string>
#include <memory>
#include <optional>

#include <dwarf.h>
#include "libdwarf.h"

#include "DIE.hpp"

// This contains the compilation units, and allows access to each of them.
// A compilation unit has a specific address range, and I could perhaps narrow
// the search for a variable by looking at the high and low PC values for each
// compilation unit [TODO].
class DwarfInfoReader
{
public:
	DwarfInfoReader(const Dwarf_Debug &dbg);

	std::vector<DIE> getCompileUnits();

	std::unique_ptr<DIE> getDIEByOffset(Dwarf_Off offset);

	std::vector<DIE> getDIEs(DIEMatcher &matcher);
	std::vector<DIE> getChildrenRecursive(DIE &die);

	struct VariableLocExpr
	{
		uint8_t frame_base;
		uint8_t location_op;
		uint8_t *location_param;
		std::unique_ptr<DIE> type;
	};
	std::optional<VariableLocExpr> getVarLocExpr(const std::string &var_name);

private:
	Dwarf_Debug dbg;
};