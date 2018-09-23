#pragma once

#include <tuple>
#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <sys/user.h>

#include <libdwarf/dwarf.h>
#include <libdwarf/libdwarf.h>

#include "../expected.hpp"
using namespace nonstd;

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
	expected<VariableLocExpr, std::string> getVarLocExpr(const std::string& var_name, pid_t pid);

private:
	Dwarf_Debug dbg;
};