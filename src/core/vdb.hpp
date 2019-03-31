#pragma once

#include <sys/types.h>
#include <unistd.h>
#include <memory>

#include "ELFFile.hpp"
#include "DebugEngine.hpp"

class VDB
{
public:
	VDB();
	~VDB();

	bool init(const char *executable_name);
	bool isInitialized();

	std::shared_ptr<ELFFile> getELFInfo();
	std::shared_ptr<DebugEngine> getDebugEngine();

private:
	bool is_initialized = false;

	bool isExecutableFile(const char *executable_name);

	std::shared_ptr<ELFFile> elf_info = nullptr;
	std::shared_ptr<DebugEngine> engine = nullptr;
};