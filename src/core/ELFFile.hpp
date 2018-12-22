#pragma once

#include <string>
#include <map>

#include "expected.hpp"

using namespace nonstd;

class ELFFile
{
public:
	ELFFile(std::string file_path);

	std::string filePath() const;
	bool hasPositionIndependentCode() const;
	expected<uint64_t, std::string> sectionAddress(const std::string& section_name) const;

private:
	std::string file_path;
	uint16_t type;
	std::map<std::string, uint64_t> section_addresses;

	uint16_t getType();
	void populateSectionAddresses();
};