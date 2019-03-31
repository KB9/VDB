#include "SharedObjectObserver.hpp"

#include <cstring>

#include <elf.h>
#include <fcntl.h>

// TODO: REMOVE
void procmsg(const char *format, ...);

SharedObjectObserver::SharedObjectObserver() :
	rendezvous_address(0),
	rendezvous_breakpoint(nullptr)
{

}

std::vector<std::string> SharedObjectObserver::getLoadedObjects(ProcessTracer& tracer, ELFFile& elf)
{
	std::vector<std::string> objects;

	// Get the rendezvous object from the target process's memory
	RendezvousPtr rendezvous = getRendezvous(tracer, elf);
	if (rendezvous == nullptr)
		return objects;

	// Read the linked list of libraries from the rendezvous map
	link_map* link_map_addr = rendezvous->r_map;
	while (link_map_addr)
	{
		uint64_t addr = reinterpret_cast<uint64_t>(link_map_addr);
		auto map = readMemoryChunk<link_map>(tracer, addr);
		uint64_t name_addr = (uint64_t)map->l_name;
		std::string name = readString(tracer, name_addr);
		link_map_addr = map->l_next;

		objects.emplace_back(name);
	}

	return objects;
}

bool SharedObjectObserver::setRendezvousBreakpoint(ProcessTracer& tracer, ELFFile& elf)
{
	RendezvousPtr rendezvous = getRendezvous(tracer, elf);
	if (rendezvous == nullptr)
		return false;

	rendezvous_breakpoint = std::make_unique<Breakpoint>(rendezvous->r_brk);
	rendezvous_breakpoint->enable(tracer);
	return true;
}

std::unique_ptr<Breakpoint>& SharedObjectObserver::getRendezvousBreakpoint()
{
	return rendezvous_breakpoint;
}

SharedObjectObserver::RendezvousPtr SharedObjectObserver::getRendezvous(ProcessTracer& tracer, ELFFile& elf)
{
	// Get the rendezvous address
	if (rendezvous_address == 0)
	{
		rendezvous_address = getRendezvousAddress(tracer, elf);
		if (rendezvous_address == 0)
		{
			procmsg("[SO_OBSERVER] Rendezvous address not yet set by dynamic linker!\n");
			return nullptr;
		}
		procmsg("[SO_OBSERVER] Rendezvous at 0x%lx\n", rendezvous_address);
	}

	// Get the rendezvous object from the target process's memory
	return readMemoryChunk<r_debug>(tracer, rendezvous_address);
}

uint64_t SharedObjectObserver::getRendezvousAddress(ProcessTracer& tracer, ELFFile& elf)
{
	// Determine the address of the .dynamic section
	auto expected_dynamic_section_address = elf.sectionAddress(".dynamic");
	assert(expected_dynamic_section_address.has_value());
	uint64_t dynamic_section_address = expected_dynamic_section_address.value();

	bool is_pie = elf.hasPositionIndependentCode();
	if (is_pie)
	{
		dynamic_section_address += tracer.tracee()->mmap().loadAddress();
	}

	uint64_t address = dynamic_section_address;
	// TODO: This can be a 32-bit word on 32-bit systems
	auto expected_word = tracer.peekText(address);
	assert(expected_word.has_value());
	uint64_t word = expected_word.value();
	while (word != DT_NULL)
	{
		if (word == DT_DEBUG)
		{
			// The address is the word after DT_DEBUG
			expected_word = tracer.peekText(address + sizeof(uint64_t));
			assert(expected_word.has_value());
			word = expected_word.value();
			return word;
		}
		address += sizeof(uint64_t);
		expected_word = tracer.peekText(address);
		assert(expected_word.has_value());
		word = expected_word.value();
	}

	return 0;
}

template <typename T>
std::unique_ptr<T> SharedObjectObserver::readMemoryChunk(ProcessTracer& tracer, uint64_t addr)
{
	std::string mem_file = "/proc/" + std::to_string(tracer.tracee()->id()) + "/mem";
	int mem_fd = open64(mem_file.c_str(), O_RDONLY);
	lseek64(mem_fd, addr, SEEK_SET);
	auto buffer = std::make_unique<T>();
	read(mem_fd, buffer.get(), sizeof(T));
	close(mem_fd);
	return buffer;
}

std::string SharedObjectObserver::readString(ProcessTracer& tracer, uint64_t start_addr)
{
	uint64_t addr = start_addr;
	std::string str = "";

	auto expected_word = tracer.peekText(addr);
	assert(expected_word.has_value());
	uint64_t word = expected_word.value();
	addr += sizeof(uint64_t);
	while (true)
	{
		auto word_ptr = reinterpret_cast<unsigned char*>(&word);

		for (int i = 0; i < 8; ++i)
		{
			if (word_ptr[i])
			{
				str += word_ptr[i];
			}
			else
			{
				start_addr = addr + i;
				return str;
			}
		}
		expected_word = tracer.peekText(addr);
		assert(expected_word.has_value());
		word = expected_word.value();
		addr += sizeof(uint64_t);
	}

	return str;
}