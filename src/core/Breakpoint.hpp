#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <unistd.h>
#include <errno.h>

#include <stdint.h>
#include <string>

class Breakpoint
{
public:
	friend class BreakpointTable;

	void *addr;
	uint64_t orig_data;
	uint64_t line_number;
	std::string file_name;

	void enable(pid_t pid);
	void disable(pid_t pid);
	bool stepOver(pid_t pid);

private:
	Breakpoint(void *addr, uint64_t line_number = 0, std::string file_name = "");
};
