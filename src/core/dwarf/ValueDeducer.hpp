#pragma once

#include <string>

#include "DwarfDebug.hpp"

class ValueDeducer
{
public:
	ValueDeducer(pid_t target_pid, std::shared_ptr<DwarfDebug> debug_data);

	std::string deduce(uint64_t address, DIE &die);

private:
	pid_t target_pid;
	std::shared_ptr<DwarfDebug> debug_data;

	std::string deduceBase(uint64_t address, const DIE &base_die);
	std::string deducePointer(uint64_t address, const DIE &pointer_die);
	std::string deduceReference(uint64_t address, const DIE &ref_die);
	std::string deduceArray(uint64_t address, DIE &array_die);
	std::string deduceStructure(uint64_t address, DIE &struct_die);
	std::string deduceClass(uint64_t address, DIE &class_die);
	std::string deduceConst(uint64_t address, const DIE &const_die);

	float decodeFloat(uint64_t data);
	double decodeDouble(uint64_t data);
};