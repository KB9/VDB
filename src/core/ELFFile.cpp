#include "ELFFile.hpp"

#include <stdint.h>
#include <gelf.h>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>

ELFFile::ELFFile(std::string path) :
	file_path(std::move(path))
{
	type = getType(file_path);
}

std::string ELFFile::filePath() const
{
	return file_path;
}

bool ELFFile::hasPositionIndependentCode() const
{
	return type == ET_DYN;
}

uint16_t ELFFile::getType(const std::string& file_path)
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