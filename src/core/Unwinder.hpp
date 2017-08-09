#pragma once

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>

#include <libunwind-ptrace.h>

class Unwinder
{
public:
	Unwinder(pid_t target_pid);
	~Unwinder();

	void unwindStep(unsigned int steps = 1);
	unw_word_t getRegisterValue(unw_regnum_t reg_num);

	void reset();

private:
	unw_cursor_t cursor;

	void *upt_info = nullptr;
	unw_addr_space_t addr_space = 0;
};