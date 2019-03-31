#pragma once

#include <memory>
#include <string>
#include <vector>

#include <link.h>

#include "Breakpoint.hpp"
#include "ELFFile.hpp"
#include "ProcessTracer.hpp"

class SharedObjectObserver
{
public:
	using RendezvousPtr = std::unique_ptr<r_debug>;

	SharedObjectObserver();

	std::vector<std::string> getLoadedObjects(ProcessTracer& tracer, ELFFile& info);

	bool setRendezvousBreakpoint(ProcessTracer& tracer, ELFFile& info);
	std::unique_ptr<Breakpoint>& getRendezvousBreakpoint();

private:
	uint64_t rendezvous_address;
	std::unique_ptr<Breakpoint> rendezvous_breakpoint;

	RendezvousPtr getRendezvous(ProcessTracer& tracer, ELFFile& info);
	uint64_t getRendezvousAddress(ProcessTracer& tracer, ELFFile& info);

	template <typename T>
	std::unique_ptr<T> readMemoryChunk(ProcessTracer& tracer, uint64_t addr);
	std::string readString(ProcessTracer& tracer, uint64_t start_addr);
};