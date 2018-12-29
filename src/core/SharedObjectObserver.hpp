#pragma once

#include <memory>
#include <string>
#include <vector>

#include <link.h>

#include "Breakpoint.hpp"
#include "ELFFile.hpp"
#include "ProcessMemoryMappings.hpp"
#include "ProcessTracer.hpp"

class SharedObjectObserver
{
public:
	using RendezvousPtr = std::unique_ptr<r_debug>;

	SharedObjectObserver();

	std::vector<std::string> getLoadedObjects(ProcessTracer& tracer,
	                                          ELFFile& elf_file,
	                                          ProcessMemoryMappings& memory_mappings);

	bool setRendezvousBreakpoint(ProcessTracer& tracer, ELFFile& elf_file,
	                             ProcessMemoryMappings& memory_mappings);

	std::unique_ptr<Breakpoint>& getRendezvousBreakpoint();

private:
	uint64_t rendezvous_address;
	std::unique_ptr<Breakpoint> rendezvous_breakpoint;

	RendezvousPtr getRendezvous(ProcessTracer& tracer, ELFFile& elf_file,
	                            ProcessMemoryMappings& memory_mappings);
	
	uint64_t getRendezvousAddress(ProcessTracer& tracer, ELFFile& elf_file,
	                              ProcessMemoryMappings& memory_mappings); 

	template <typename T>
	std::unique_ptr<T> readMemoryChunk(ProcessTracer& tracer, uint64_t addr);
	std::string readString(ProcessTracer& tracer, uint64_t start_addr);
};