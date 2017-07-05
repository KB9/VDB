#pragma once

#include <string>

#include "dwarf/DIEBaseType.hpp"
#include "dwarf/DIEPointerType.hpp"
#include "dwarf/DwarfDebug.hpp"

class ValueDeducer
{
public:
	ValueDeducer(pid_t target_pid, std::shared_ptr<DwarfDebug> debug_data);

	std::string deduce(uint64_t address, const DIEBaseType &base_die);
	std::string deduce(uint64_t address, const DIEPointerType &pointer_die);

private:
	pid_t target_pid;
	std::shared_ptr<DwarfDebug> debug_data;
};