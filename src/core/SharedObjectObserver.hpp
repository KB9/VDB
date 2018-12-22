#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Breakpoint.hpp"
#include "ELFFile.hpp"
#include "ProcessMemoryMappings.hpp"
#include "ProcessTracer.hpp"

class SharedObjectObserver
{
public:
	SharedObjectObserver();

	std::vector<std::string> getLoadedObjects(ProcessTracer& tracer,
	                                          ELFFile& elf_file,
	                                          ProcessMemoryMappings& memory_mappings);

private:
	uint64_t rendezvous_address;
	uint64_t library_update_address;

	uint64_t getRendezvousAddress(ProcessTracer& tracer, ELFFile& elf_file,
	                              ProcessMemoryMappings& memory_mappings);

	template <typename T>
	std::unique_ptr<T> readMemoryChunk(ProcessTracer& tracer, uint64_t addr);
	std::string readString(ProcessTracer& tracer, uint64_t start_addr);
};