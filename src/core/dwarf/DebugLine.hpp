#pragma once

#include "libdwarf.h"
#include <dwarf.h>

#include <unordered_map>
#include <stdint.h>
#include <memory>
#include <optional>

#include "DIE.hpp"

// Structure containing information about a single source line
struct Line
{
	Line(uint64_t number, uint64_t address, bool is_begin_statement, char *source) :
		number(number),
		address(address),
		is_begin_statement(is_begin_statement),
		source(source) {}

	const uint64_t number;
	const uint64_t address;
	const bool is_begin_statement;
	const char *source;
};

// Represents the information presented when performing a call to
// objdump --dwarf=rawline/decodedline
class DebugLine
{
public:
	DebugLine(const std::vector<DIE> &compile_units);

	// std::optional<Line> getLine(uint64_t address);
	std::vector<Line> getFunctionLines(uint64_t address);
	std::vector<Line> getCULines(uint64_t address);
	std::vector<Line> getCULines(const DIE &compile_unit);

private:
	std::vector<DIE> compile_units;

	std::optional<DIE> getCompileUnit(uint64_t address);
	std::vector<Line> generateLineInfo(const DIE &compile_unit,
	                                   uint64_t start_address = 0,
	                                   uint64_t end_address = 0);
};