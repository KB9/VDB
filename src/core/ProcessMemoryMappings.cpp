#include "ProcessMemoryMappings.hpp"

#include <fstream>
#include <string>

void procmsg(const char* format, ...);

ProcessMemoryMappings::ProcessMemoryMappings(pid_t pid)
{
	load_address = readLoadAddressFromProcMaps(pid);
}

uint64_t ProcessMemoryMappings::loadAddress() const
{
	return load_address;
}

uint64_t ProcessMemoryMappings::readLoadAddressFromProcMaps(pid_t pid)
{
	std::string pmaps_path = "/proc/" + std::to_string(pid) + "/maps";

	// DEBUG: Print out the entire file for debugging purposes
	std::ifstream file(pmaps_path);
	std::string line;
	std::string first_line;
	while (std::getline(file, line))
	{
		if (first_line.empty())
			first_line = line;
		procmsg("%s\n", line.c_str());
	}

	std::string load_addr_str = first_line.substr(0, first_line.find("-"));
	uint64_t load_addr = std::stoull(load_addr_str, 0, 16);
	return load_addr;
}