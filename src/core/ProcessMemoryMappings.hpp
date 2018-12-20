#pragma once

#include <sys/types.h>

#include <stdint.h>

class ProcessMemoryMappings
{
public:
	ProcessMemoryMappings(pid_t pid);

	uint64_t loadAddress() const;

private:
	uint64_t load_address;

	uint64_t readLoadAddressFromProcMaps(pid_t pid);
};