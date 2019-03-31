#pragma once

#include "ProcessMemoryMappings.hpp"

class Process
{
public:
	Process(pid_t process_id);

	pid_t id() const;
	const ProcessMemoryMappings& mmap() const;

private:
	pid_t pid;
	ProcessMemoryMappings memory_mappings;
};