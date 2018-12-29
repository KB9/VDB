#include "ELFFile.hpp"

#include <stdint.h>
#include <gelf.h>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>

ELFFile::ELFFile(std::string path) :
	file_path(std::move(path))
{
	type = getType();
	entry_point = getEntryPoint();
	populateSectionAddresses();
}

std::string ELFFile::filePath() const
{
	return file_path;
}

uint64_t ELFFile::entryPoint() const
{
	return entry_point;
}

bool ELFFile::hasPositionIndependentCode() const
{
	return type == ET_DYN;
}

expected<uint64_t, std::string> ELFFile::sectionAddress(const std::string& section_name) const
{
	auto it = section_addresses.find(section_name);
	bool found = (it != std::end(section_addresses));
	if (found)
	{
		return it->second;
	}
	else
	{
		std::string error = "Section " + section_name + " not found in ELF file: " + file_path;
		return make_unexpected(error);
	}
}

uint64_t ELFFile::getEntryPoint()
{
	// Ensure the ELF library initialization doesn't fail
	assert(elf_version(EV_CURRENT) != EV_NONE);

	// Ensure the executable file can be read successfully
	int fd = open(file_path.c_str(), O_RDONLY, 0);
	assert(fd >= 0);

	Elf* elf = elf_begin(fd, ELF_C_READ, NULL);
	assert(elf != NULL);

	// Ensure the executable is an ELF object
	assert(elf_kind(elf) == ELF_K_ELF);

	GElf_Ehdr elf_header;
	if (gelf_getehdr(elf, &elf_header) == NULL)
	{
		assert(false);
	}

	uint64_t entry = elf_header.e_entry;

	elf_end(elf);
	close(fd);

	return entry;
}

uint16_t ELFFile::getType()
{
	// Ensure the ELF library initialization doesn't fail
	assert(elf_version(EV_CURRENT) != EV_NONE);

	// Ensure the executable file can be read successfully
	int fd = open(file_path.c_str(), O_RDONLY, 0);
	assert(fd >= 0);

	Elf* elf = elf_begin(fd, ELF_C_READ, NULL);
	assert(elf != NULL);

	// Ensure the executable is an ELF object
	assert(elf_kind(elf) == ELF_K_ELF);

	GElf_Ehdr elf_header;
	if (gelf_getehdr(elf, &elf_header) == NULL)
	{
		assert(false);
	}

	uint16_t file_type = elf_header.e_type;

	elf_end(elf);
	close(fd);

	return file_type;
}

void ELFFile::populateSectionAddresses()
{
	// Ensure the ELF library initialization doesn't fail
	assert(elf_version(EV_CURRENT) != EV_NONE);

	// Ensure the executable file can be read successfully
	int fd = open(file_path.c_str(), O_RDONLY, 0);
	assert(fd >= 0);

	Elf* elf = elf_begin(fd, ELF_C_READ, NULL);
	assert(elf != NULL);

	// Ensure the executable is an ELF object
	assert(elf_kind(elf) == ELF_K_ELF);

	size_t shstrndx;
	assert(elf_getshdrstrndx(elf, &shstrndx) == 0);

	Elf_Scn* elf_section = NULL;
	while ((elf_section = elf_nextscn(elf, elf_section)) != NULL)
	{
		GElf_Shdr elf_section_header;
		if (gelf_getshdr(elf_section, &elf_section_header) != &elf_section_header)
		{
			assert(false);
		}

		char *name;
		if ((name = elf_strptr(elf, shstrndx, elf_section_header.sh_name)) == NULL)
		{
			assert(false);
		}

		section_addresses.emplace(name, elf_section_header.sh_addr);
	}

	elf_end(elf);
	close(fd);
}