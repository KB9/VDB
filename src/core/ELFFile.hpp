#pragma once

#include <string>

class ELFFile
{
public:
	ELFFile(std::string file_path);

	std::string filePath() const;
	bool hasPositionIndependentCode() const;

private:
	std::string file_path;
	uint16_t type;

	uint16_t getType(const std::string& file_path);
};