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

	auto loc_expr = dwarf->info()->getVarLocExpr(variable_name);
	DwarfExprInterpreter interpreter(pid);
	uint64_t address = interpreter.parse(&loc_expr.frame_base,
	                                     loc_expr.location_op,
	                                     loc_expr.location_param);
	if (address > 0)
	{
		ValueDeducer deducer(pid, dwarf);
		var.value = deducer.deduce(address, *(loc_expr.type));
	}
	else
	{
		var.value = "Variable not locatable";
	}
	return var;
}

DwarfDebugInfo::Function DwarfDebugInfo::getFunction(uint64_t address) const
{
	DIEMatcher matcher;
	matcher.setTags({"DW_TAG_subprogram"});
	std::vector<DIE> subprograms = dwarf->info()->getDIEs(matcher);
	for (auto sub : subprograms)
	{
		// TODO: Subprogram DIEs do not have to have lowpc/highpc address values.
		// This occurs when they are externally defined from the CU.
		uint64_t low_pc = sub.getAttributeByCode(DW_AT_low_pc).getAddress();
		uint64_t high_pc = sub.getAttributeByCode(DW_AT_high_pc).getOffset();
		if (address >= low_pc && address < (low_pc + high_pc))
		{
			DebugInfo::Function function;
			// function.name = sub.getTagName();
			function.name = sub.getAttributeByCode(DW_AT_name).getString();
			function.start_address = sub.getAttributeByCode(DW_AT_low_pc).getAddress();
			function.end_address = sub.getAttributeByCode(DW_AT_low_pc).getAddress() + sub.getAttributeByCode(DW_AT_high_pc).getOffset();

			Dwarf_Off cu_offset = sub.getCUOffset();
			DIE cu = *(dwarf->info()->getDIEByOffset(cu_offset));
			Attribute file_name = cu.getAttributeByCode(DW_AT_name);
			Attribute file_dir = cu.getAttributeByCode(DW_AT_comp_dir);
			function.decl_file = file_dir.getString() + "/" + file_name.getString();

			function.decl_line = sub.getAttributeByCode(DW_AT_decl_line).getUnsigned();

			return function;
		}
	}

	assert(false && "Address not within a function");
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