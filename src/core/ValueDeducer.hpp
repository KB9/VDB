#pragma once

#include <string>

#include "dwarf/DIEBaseType.hpp"
#include "dwarf/DIEPointerType.hpp"

class ValueDeducer
{
public:
	ValueDeducer(pid_t target_pid);

	std::string deduce(uint64_t address, const DIEBaseType &base_die);
	std::string deduce(uint64_t address, const DIEPointerType &pointer_die);

private:
	pid_t target_pid;
};