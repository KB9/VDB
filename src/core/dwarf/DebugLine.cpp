#include "DebugLine.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

DebugLine::DebugLine(const std::vector<DIE> &compile_units) :
	compile_units(compile_units)
{

}

// std::optional<Line> DebugLine::getLine(uint64_t address)
// {
// 	DIE compile_unit = getCompileUnit(address);
// 	std::vector<Line> lines = generateLineInfo(compile_unit, address, address + 1);
// 	return (lines.empty() ? std::nullopt : std::make_optional<Line>(lines.at(0)));
// }

std::vector<Line> DebugLine::getFunctionLines(uint64_t address)
{
	auto cu_expected = getCompileUnit(address);
	if (!cu_expected)
		return {};

	DIE compile_unit = cu_expected.value();
	std::vector<DIE> children = compile_unit.getChildren();
	while (!children.empty())
	{
		// Examine all this child's children
		std::vector<DIE> subchildren = children.front().getChildren();
		children.insert(children.end(), subchildren.begin(), subchildren.end());

		// If the child is a subprogram
		if (children.front().getTagName() == "DW_TAG_subprogram")
		{
			// If the subprogram has address range information
			auto low_pc_opt = children.front().getAttributeValue<DW_AT_low_pc>();
			auto high_pc_opt = children.front().getAttributeValue<DW_AT_high_pc>();
			if (low_pc_opt.has_value() && high_pc_opt.has_value())
			{
				// Check if the address is within this function's range
				Dwarf_Addr low_pc = low_pc_opt.value();
				Dwarf_Off high_pc = high_pc_opt.value();
				if (address >= low_pc && address < (low_pc + high_pc))
				{
					std::vector<Line> func_lines = generateLineInfo(compile_unit,
					                                                low_pc,
					                                                (low_pc + high_pc));
					return func_lines;
				}
			}
		}

		children.erase(children.begin());
	}
	return {};
}

std::vector<Line> DebugLine::getCULines(uint64_t address)
{
	auto cu_expected = getCompileUnit(address);
	if (!cu_expected)
		return generateLineInfo(cu_expected.value());
	else
		return {};
}

std::vector<Line> DebugLine::getCULines(const DIE &compile_unit)
{
	return generateLineInfo(compile_unit);
}

expected<DIE, std::string> DebugLine::getCompileUnit(uint64_t address)
{
	for (const auto& cu : compile_units)
	{
		auto expected_low_pc = cu.getAttributeValue<DW_AT_low_pc>();
		auto expected_high_pc = cu.getAttributeValue<DW_AT_high_pc>();
		if (!expected_low_pc || !expected_high_pc)
			continue;

		Dwarf_Addr low_pc = expected_low_pc.value();
		Dwarf_Off high_pc = expected_high_pc.value();
		if (address >= low_pc && address < (low_pc + high_pc))
			return cu;
	}
	return make_unexpected("Failed to find compilation unit at address: " + std::to_string(address));
}

std::vector<Line> DebugLine::generateLineInfo(const DIE &compile_unit,
                                              uint64_t start_address,
                                              uint64_t end_address)
{
	/*
	TODO:
	This function uses the DWARF2,3,4 style for getting line information.
	DWARF5 has more descriptive line information functions and should be
	implemented as well.
	*/

	std::vector<Line> lines;

	Dwarf_Line *line_buffer = nullptr;
	Dwarf_Signed line_count = 0;
	Dwarf_Error err;

	// Get the source lines from the specified compilation unit
	int result = dwarf_srclines(
		compile_unit.get(),
		&line_buffer,
		&line_count,
		&err);

	if (result == DW_DLV_OK)
	{
		for (int i = 0; i < line_count; i++)
		{
			Dwarf_Error err;

			// Get the line address
			Dwarf_Addr line_addr;
			result = dwarf_lineaddr(line_buffer[i], &line_addr, &err);
			if (result != DW_DLV_OK)
				procmsg("[DWARF_ERROR] Error in dwarf_lineaddr!\n");

			// Only include the line entries which are within the specified
			// address range
			if (start_address > 0 && end_address > 0)
				if (line_addr < start_address || line_addr >= end_address)
					continue;

			// Get the line number
			Dwarf_Unsigned line_number;
			int result = dwarf_lineno(line_buffer[i], &line_number, &err);
			if (result != DW_DLV_OK)
				procmsg("[DWARF_ERROR] Error in dwarf_lineno!\n");

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

			// Store the line information
			lines.emplace_back(line_number, line_addr, is_begin_statement, line_src);
		}
	}
	else
	{
		procmsg("[DWARF_ERROR] Error in dwarf_srclines!\n");
	}
	return lines;
}