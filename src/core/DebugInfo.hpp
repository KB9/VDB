#ifndef _DEBUG_INFO_H_
#define _DEBUG_INFO_H_

#include <string>
#include <vector>
#include <memory>

#include <sys/types.h>

class DwarfDebug;

/*
This is a unified and simplified interface for retrieving information about
debugging information, instead of manipulating DWARF information outside of the
dwarf subfolder (including having to worry about the differences in information
between DWARF versions).
*/

class DebugInfo
{
public:
	struct Function
	{
		std::string name;
		uint64_t start_address;
		uint64_t end_address;
		std::string decl_file;
		uint64_t decl_line;
	};

	struct Variable
	{
		std::string name;
		std::string value;
	};

	struct SourceLine
	{
		uint64_t number;
		uint64_t address;
		std::string file_name;
	};

	static std::shared_ptr<DebugInfo> readFrom(const std::string &executable_name);

	virtual Variable getVariable(const std::string &variable_name, pid_t pid) const = 0;
	virtual Function getFunction(uint64_t address) const = 0;
	virtual std::vector<SourceLine> getAllLines() const = 0;
	virtual std::vector<std::string> getSourceFiles() const = 0;
};

class DwarfDebugInfo : public DebugInfo
{
public:
	DwarfDebugInfo(const std::string &executable_name);

	virtual Variable getVariable(const std::string &variable_name, pid_t pid) const override;
	virtual Function getFunction(uint64_t address) const override;
	virtual std::vector<SourceLine> getAllLines() const override;
	virtual std::vector<std::string> getSourceFiles() const override;

private:
	std::shared_ptr<DwarfDebug> dwarf = nullptr;
};

#endif // _DEBUG_INFO_H_