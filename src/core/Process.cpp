#include "Process.hpp"

Process::Process(pid_t process_id) :
	pid(process_id),
	memory_mappings(process_id)
{

}

pid_t Process::id() const
{
	return pid;
}

const ProcessMemoryMappings& Process::mmap() const
{
	return memory_mappings;
}