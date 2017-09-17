#pragma once

#include <string>

#include "dwarf/DIEBaseType.hpp"
#include "dwarf/DIEPointerType.hpp"
#include "dwarf/DIEArrayType.hpp"
#include "dwarf/DIEStructureType.hpp"
#include "dwarf/DIEClassType.hpp"
#include "dwarf/DIEConstType.hpp"
#include "dwarf/DwarfDebug.hpp"

class ValueDeducer
{
public:
	ValueDeducer(pid_t target_pid, std::shared_ptr<DwarfDebug> debug_data);

	std::string deduce(uint64_t address, DebuggingInformationEntry &die);

private:
	pid_t target_pid;
	std::shared_ptr<DwarfDebug> debug_data;

	std::string deduceBase(uint64_t address, const DIEBaseType &base_die);
	std::string deducePointer(uint64_t address, const DIEPointerType &pointer_die);
	std::string deduceArray(uint64_t address, DIEArrayType &array_die);
	std::string deduceStructure(uint64_t address, DIEStructureType &struct_die);
	std::string deduceClass(uint64_t address, DIEClassType &class_die);
	std::string deduceConst(uint64_t address, DIEConstType &const_die);
};