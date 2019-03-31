#pragma once

#include <string>
#include <memory>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <unistd.h>

#include "Process.hpp"

#include "expected.hpp"

using namespace nonstd;

// A wrapper for ptrace
class ProcessTracer
{
public:
	using Signal = int;
	using Address = uint64_t;
	using Text = uint64_t;

	template <class T>
	using Result = expected<T, std::string>;

	ProcessTracer();
	ProcessTracer(const ProcessTracer&) = delete;
	ProcessTracer(ProcessTracer&& other);

	bool start(const std::string& executable_path);

	Result<Signal> continueExec();
	Result<Signal> singleStepExec();

	Result<user_regs_struct> getRegisters();
	Result<void> setRegisters(const user_regs_struct& regs);

	Result<Text> peekText(Address address);
	Result<void> pokeText(Address address, Text text);

	const std::unique_ptr<Process>& tracee() const;
	bool isStopped() const;
	bool isRunning() const;

	ProcessTracer& operator=(const ProcessTracer&) = delete;
	ProcessTracer& operator=(ProcessTracer&& other);

private:
	std::unique_ptr<Process> process;
	bool is_stopped;
	bool is_running;

	Result<void> runTarget(const std::string& executable_path);

	Result<Signal> wait();
};