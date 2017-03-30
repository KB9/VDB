#include "DebugLine.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

DebugLine::DebugLine(DIECompileUnit &compile_unit_die)
{
	/*
	TODO:
	This function uses the DWARF2,3,4 style for getting line information.
	DWARF5 has more descriptive line information functions and should be
	implemented as well.
	*/

	Dwarf_Line *line_buffer = nullptr;
	Dwarf_Signed line_count = 0;
	Dwarf_Error err;

	// Get the source lines from the specified compilation unit
	int result = dwarf_srclines(
		compile_unit_die.getInternalDie(),
		&line_buffer,
		&line_count,
		&err);

	if (result == DW_DLV_OK)
	{
		for (int i = 0; i < line_count; i++)
		{
			Dwarf_Error err;

			// Get the line number
			Dwarf_Unsigned line_number;
			int result = dwarf_lineno(line_buffer[i], &line_number, &err);
			if (result != DW_DLV_OK)
				procmsg("[DWARF_ERROR] Error in dwarf_lineno!\n");

			// Get the line address
			Dwarf_Addr line_addr;
			result = dwarf_lineaddr(line_buffer[i], &line_addr, &err);
			if (result != DW_DLV_OK)
				procmsg("[DWARF_ERROR] Error in dwarf_lineaddr!\n");

			// Determine whether it is the beginning statement or not
			Dwarf_Bool is_begin_statement;
			result = dwarf_linebeginstatement(line_buffer[i], &is_begin_statement, &err);
			if (result != DW_DLV_OK)
				procmsg("[DWARF_ERROR] Error in dwarf_linebeginstatement!\n");

			// Get the source file the line came from
			char *line_src;
			dwarf_linesrc(line_buffer[i], &line_src, &err);
			if (result != DW_DLV_OK)
				procmsg("[DWARF_ERROR] Error in dwarf_linesrc!\n");

			// Store the line information and insert it into the lines map
			Line line(line_number, line_addr, is_begin_statement, line_src);
			insertLine(line);

			procmsg("[DWARF] [%s] Line %d saved! (0x%08x)\n", line_src, line_number, line_addr);
		}
	}
	else
	{
		procmsg("[DWARF_ERROR] Error in dwarf_srclines!\n");
	}
}

void DebugLine::insertLine(const Line &line)
{
	// Push all lines to the line vector
	line_vector.push_back(line);

	// Check if a line is already inserted at this line number in the map
	std::unordered_map<uint64_t, std::vector<Line>>::const_iterator it = line_map.find(line.number);
	if (it == line_map.end())
	{
		// If the line is not already in the map
		// Create a new vector, insert the line and insert the vector to the map
		std::vector<Line> lines;
		lines.push_back(line);
		line_map.insert(std::pair<uint64_t, std::vector<Line>>(line.number, lines));
	}
	else
	{
		// Insert the line into the element's line vector
		std::vector<Line> &lines = line_map.at(line.number);
		lines.push_back(line);
	}
}

// Get the line information for the line at the specified line number
std::unique_ptr<std::vector<Line>> DebugLine::getLine(uint64_t line_number)
{
	std::unordered_map<uint64_t, std::vector<Line>>::const_iterator it = line_map.find(line_number);
	if (it == line_map.end())
	{
		return nullptr;
	}
	else
	{
		return std::make_unique<std::vector<Line>>(line_map.at(line_number));
	}
}

// Gets a vector of all the lines
std::vector<Line> DebugLine::getAllLines()
{
	return line_vector;
}