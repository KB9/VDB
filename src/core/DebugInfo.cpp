#include "DebugInfo.hpp"

#include <cassert>

#include "dwarf/DwarfDebug.hpp"
#include "dwarf/DwarfExprInterpreter.hpp"
#include "dwarf/ValueDeducer.hpp"

std::string DebugInfo::toAbsolutePath(const std::string &dir, const std::string &name)
{
	// If the file path is relative, the directory it was compiled within
	// will become the prefix. If not, only the name is used as it defines
	// the fully-qualified path.
	bool is_relative_path = (name.at(0) != '/');
	if (is_relative_path)
		return dir + '/' + name;
	else
		return name;
}

DwarfDebugInfo::DwarfDebugInfo(const std::string &executable_name) :
	dwarf(std::make_shared<DwarfDebug>(executable_name))
{

}

DwarfDebugInfo::Variable DwarfDebugInfo::getVariable(const std::string &variable_name, pid_t pid) const
{
	DwarfDebugInfo::Variable var;
	var.name = variable_name;

	auto loc_expr_opt = dwarf->info()->getVarLocExpr(variable_name, pid);
	if (loc_expr_opt.has_value())
	{
		DwarfExprInterpreter interpreter(pid);
		uint64_t address = interpreter.parse(&loc_expr_opt.value().frame_base,
		                                     loc_expr_opt.value().location_op,
		                                     loc_expr_opt.value().location_param);
		if (address > 0)
		{
			ValueDeducer deducer(pid, dwarf);
			var.value = deducer.deduce(address, *(loc_expr_opt.value().type));
		}
		else
		{
			var.value = "Variable not locatable";
		}
	}
	else
	{
		var.value = "Variable not locatable";
	}
	return var;
}

expected<DwarfDebugInfo::Function, std::string> DwarfDebugInfo::getFunction(uint64_t address) const
{
	DIEMatcher matcher;
	matcher.setTags({"DW_TAG_subprogram"});
	std::vector<DIE> subprograms = dwarf->info()->getDIEs(matcher);
	for (auto sub : subprograms)
	{
		auto expected_low_pc = sub.getAttributeValue<DW_AT_low_pc>();
		auto expected_high_pc = sub.getAttributeValue<DW_AT_high_pc>();

		// Subprogram DIEs may not have lowpc/highpc address values.
		// This occurs when they are externally defined from the CU.
		if (!expected_low_pc || !expected_high_pc)
			continue;

		uint64_t low_pc = expected_low_pc.value();
		uint64_t high_pc = expected_high_pc.value();
		if (address >= low_pc && address < (low_pc + high_pc))
		{
			DebugInfo::Function function;
			function.name = sub.getAttributeValue<DW_AT_name>().value();
			function.start_address = sub.getAttributeValue<DW_AT_low_pc>().value();
			function.end_address = sub.getAttributeValue<DW_AT_low_pc>().value() + sub.getAttributeValue<DW_AT_high_pc>().value();

			Dwarf_Off cu_offset = sub.getCUOffset();
			DIE cu = *(dwarf->info()->getDIEByOffset(cu_offset));
			std::string file_name = cu.getAttributeValue<DW_AT_name>().value();
			std::string file_dir = cu.getAttributeValue<DW_AT_comp_dir>().value();
			function.decl_file = toAbsolutePath(file_dir, file_name);
			function.decl_line = sub.getAttributeValue<DW_AT_decl_line>().value();

			return function;
		}
	}
	return make_unexpected("Failed to find function at address: " + std::to_string(address));
}

// std::optional<DwarfDebugInfo::SourceLine> DwarfDebugInfo::getLine(uint64_t address) const
// {
// 	std::optional<Line> line = dwarf->line()->getLine(address);
// 	if (line.has_value())
// 	{
// 		Line val = line.value();
// 		return std::optional<DwarfDebugInfo::SourceLine>({val.number, val.address, val.source});
// 	}
// 	else
// 	{
// 		return std::nullopt;
// 	}
// }

std::vector<DwarfDebugInfo::SourceLine> DwarfDebugInfo::getFunctionLines(uint64_t address) const
{
	std::vector<Line> dwarf_lines = dwarf->line()->getFunctionLines(address);
	std::vector<DwarfDebugInfo::SourceLine> lines;
	for (const auto &line : dwarf_lines)
		lines.push_back({line.number, line.address, line.source});
	return lines;
}

std::vector<DwarfDebugInfo::SourceLine> DwarfDebugInfo::getSourceFileLines(const std::string &file_name) const
{
	std::vector<DIE> compile_units = dwarf->info()->getCompileUnits();
	for (const auto &cu : compile_units)
	{
		std::string name = cu.getAttributeValue<DW_AT_name>().value();
		std::string dir = cu.getAttributeValue<DW_AT_comp_dir>().value();
		std::string path = toAbsolutePath(dir, name);

		if (file_name == path)
		{
			std::vector<Line> dwarf_lines = dwarf->line()->getCULines(cu);
			std::vector<DwarfDebugInfo::SourceLine> lines;
			for (const auto &line : dwarf_lines)
				lines.push_back({line.number, line.address, line.source});
			return lines;
		}
	}
	return {};
}

std::vector<std::string> DwarfDebugInfo::getSourceFiles() const
{
	std::vector<std::string> file_names;

	std::vector<SourceFile> files = sourceFiles(dwarf);
	for (const auto &file : files)
	{
		file_names.push_back(toAbsolutePath(file.dir, file.name));
	}
	return file_names;
}