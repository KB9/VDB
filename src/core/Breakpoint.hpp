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

#include "ProcessTracer.hpp"

class Breakpoint
{
public:
	friend class BreakpointTable;

	uint64_t addr;
	uint64_t orig_data;

	Breakpoint(const Breakpoint &other) = delete;
	Breakpoint(Breakpoint &&other);

	void enable(ProcessTracer& tracer);
	void disable(ProcessTracer& tracer);
	bool stepOver(ProcessTracer& tracer);

	Breakpoint &operator=(const Breakpoint &other) = delete;
	Breakpoint &operator=(Breakpoint &&other);

private:
	Breakpoint(uint64_t addr);
};
