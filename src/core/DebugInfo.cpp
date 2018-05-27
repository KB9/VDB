#include "DebugInfo.hpp"

#include <cassert>

#include "dwarf/DwarfDebug.hpp"
#include "dwarf/DwarfExprInterpreter.hpp"
#include "dwarf/ValueDeducer.hpp"

std::shared_ptr<DebugInfo> DebugInfo::readFrom(const std::string &executable_name)
{
	return std::make_shared<DwarfDebugInfo>(executable_name);
}

DwarfDebugInfo::DwarfDebugInfo(const std::string &executable_name) :
	dwarf(std::make_shared<DwarfDebug>(executable_name))
{

}

DwarfDebugInfo::Variable DwarfDebugInfo::getVariable(const std::string &variable_name, pid_t pid) const
{
	DwarfDebugInfo::Variable var;
	var.name = variable_name;

	auto loc_expr_opt = dwarf->info()->getVarLocExpr(variable_name);
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

std::optional<DwarfDebugInfo::Function> DwarfDebugInfo::getFunction(uint64_t address) const
{
	DIEMatcher matcher;
	matcher.setTags({"DW_TAG_subprogram"});
	std::vector<DIE> subprograms = dwarf->info()->getDIEs(matcher);
	for (auto sub : subprograms)
	{
		auto low_pc_opt = sub.getAttributeValue<DW_AT_low_pc>();
		auto high_pc_opt = sub.getAttributeValue<DW_AT_high_pc>();

		// Subprogram DIEs may not have lowpc/highpc address values.
		// This occurs when they are externally defined from the CU.
		if (!low_pc_opt.has_value() || !high_pc_opt.has_value())
			continue;

		uint64_t low_pc = low_pc_opt.value();
		uint64_t high_pc = high_pc_opt.value();
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
			function.decl_file = file_dir + "/" + file_name;
			function.decl_line = sub.getAttributeValue<DW_AT_decl_line>().value();

			return std::make_optional<DwarfDebugInfo::Function>(function);
		}
	}
	return std::nullopt;
}

std::vector<DwarfDebugInfo::SourceLine> DwarfDebugInfo::getAllLines() const
{
	std::vector<DebugInfo::SourceLine> all_lines;
	for (Line line : dwarf->line()->getAllLines())
	{
		all_lines.push_back({line.number, line.address, line.source});
	}
	return all_lines;
}

std::vector<std::string> DwarfDebugInfo::getSourceFiles() const
{
	std::vector<std::string> file_names;

	std::vector<SourceFile> files = sourceFiles(dwarf);
	for (const auto &file : files)
	{
		// If the file path is relative, the directory it was compiled within
		// will become the prefix. If not, only the name is used as it defines
		// the fully-qualified path.
		bool is_relative_path = (file.name.at(0) != '/');
		if (is_relative_path)
			file_names.push_back(file.dir + '/' + file.name);
		else
			file_names.push_back(file.name);
	}
	return file_names;
}