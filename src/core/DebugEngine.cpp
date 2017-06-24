#include "DebugEngine.hpp"

#include "dwarf/CUHeader.hpp"
#include "dwarf/DIEVariable.hpp"

#include <string>

DebugEngine::DebugEngine(const char *executable_name, std::shared_ptr<DwarfDebug> debug_data) :
	debug_data(debug_data)
{
	if (target_name != NULL) delete target_name;
	target_name = new char[strlen(executable_name) + 1];
	strcpy(target_name, executable_name);

	breakpoint_table = std::make_shared<BreakpointTable>(debug_data);
}

DebugEngine::~DebugEngine()
{
	if (target_name != NULL) delete target_name;
	target_name = NULL;
}

bool DebugEngine::run(BreakpointCallback breakpoint_callback)
{
	debugger = std::make_shared<ProcessDebugger>(target_name, breakpoint_table, breakpoint_callback);
	return true;
}

std::shared_ptr<BreakpointTable> DebugEngine::getBreakpoints()
{
	return breakpoint_table;
}

char *DebugEngine::getValue(const char *variable_name)
{
	procmsg("[GET_VALUE] Attempting to get value of variable '%s'...\n", variable_name);

	// Get the first location expression for the specified variable name
	VariableLocExpr loc_expr;
	bool found = false;
	for (CUHeader header : debug_data->info()->getCUHeaders())
	{
		auto loc_exprs = header.getLocExprsFromVarName<DIEVariable>(variable_name);
		if (loc_exprs.size() > 0)
		{
			loc_expr = loc_exprs.at(0);
			found = true;
			break;
		}
	}

	// If no results were found for the specified variable name, return
	if (!found) return nullptr;

	// Retrieve the value of the specified variable
	char *deduced_value = nullptr;
	debugger->getValue(loc_expr,
	                   debug_data->info()->getDIEByOffset(loc_expr.type_die_offset).get(),
	                   &deduced_value);

	// Wait until the deduced value has been set by the debugger
	while (deduced_value == nullptr) {}

	return deduced_value;
}