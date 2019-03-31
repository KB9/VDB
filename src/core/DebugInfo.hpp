#ifndef _DEBUG_INFO_H_
#define _DEBUG_INFO_H_

#include <string>
#include <vector>
#include <memory>

#include <sys/types.h>

#include "expected.hpp"
using namespace nonstd;

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

	virtual Variable getVariable(const std::string &variable_name, pid_t pid) const = 0;
	virtual expected<Function, std::string> getFunction(uint64_t address) const = 0;
	// virtual std::optional<SourceLine> getLine(uint64_t address) const = 0;
	virtual std::vector<SourceLine> getFunctionLines(uint64_t address) const = 0;
	virtual std::vector<SourceLine> getSourceFileLines(const std::string &file_name) const = 0;
	virtual std::vector<std::string> getSourceFiles() const = 0;

	static std::string toAbsolutePath(const std::string &dir, const std::string &file);
};

class DwarfDebugInfo : public DebugInfo
{
public:
	DwarfDebugInfo(const std::string &executable_name);

	virtual Variable getVariable(const std::string &variable_name, pid_t pid) const override;
	virtual expected<Function, std::string> getFunction(uint64_t address) const override;
	// virtual std::optional<SourceLine> getLine(uint64_t address) const override;
	virtual std::vector<SourceLine> getFunctionLines(uint64_t address) const override;
	virtual std::vector<SourceLine> getSourceFileLines(const std::string &file_name) const override;
	virtual std::vector<std::string> getSourceFiles() const override;

private:
	std::shared_ptr<DwarfDebug> dwarf = nullptr;
};

#endif // _DEBUG_INFO_H_