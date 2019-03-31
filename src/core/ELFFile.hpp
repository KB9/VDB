#pragma once

#include <string>
#include <map>

#include "DebugInfo.hpp"

#include "expected.hpp"
using namespace nonstd;

class ELFFile
{
public:
	ELFFile(std::string file_path);

	std::string filePath() const;
	uint64_t entryPoint() const;
	bool hasPositionIndependentCode() const;
	expected<uint64_t, std::string> sectionAddress(const std::string& section_name) const;

	const std::shared_ptr<DebugInfo>& debugInfo() const;

private:
	std::string file_path;
	uint64_t entry_point;
	uint16_t type;
	std::map<std::string, uint64_t> section_addresses;
	std::shared_ptr<DebugInfo> debug_info;

	uint64_t getEntryPoint();
	uint16_t getType();
	void populateSectionAddresses();
};