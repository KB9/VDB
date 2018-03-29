#pragma once

#include "libdwarf.h"
#include <dwarf.h>

#include <unordered_map>
#include <stdint.h>
#include <memory>

// #include "DIECompileUnit.hpp"
#include "DwarfReader.hpp"

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
// TODO: Should hold line info for all compilation units - not just the one in the constructor
class DebugLine
{
public:
	// DebugLine(DIECompileUnit &compile_unit_die);
	DebugLine(DIE &compile_unit_die);

	std::unique_ptr<std::vector<Line>> getLine(uint64_t line_number);
	std::vector<Line> getAllLines();

private:
	std::unordered_map<uint64_t, std::vector<Line>> line_map;
	std::vector<Line> line_vector;

	void insertLine(const Line &line);
};