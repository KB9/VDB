#pragma once

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>

#include <libunwind-ptrace.h>

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

class Backtracer
{
public:
	Backtracer(pid_t target_pid)
	{
		addr_space = unw_create_addr_space(&_UPT_accessors, 0);
		if (!addr_space)
		{
			procmsg("[UNWIND_ERROR] unw_create_addr_space failed!\n");
			return;
		}

		upt_info = _UPT_create(target_pid);
		if (!upt_info)
		{
			procmsg("[UNWIND_ERROR] System is out of memory!\n");
			unw_destroy_addr_space(addr_space);
			return;
		}
	}

	~Backtracer()
	{
		_UPT_destroy(upt_info);
		unw_destroy_addr_space(addr_space);
	}

	void backtrace()
	{
		unw_cursor_t cursor;
		int result = unw_init_remote(&cursor, addr_space, upt_info);
		if (result < 0)
			procmsg("[UNWIND_ERROR] unw_init_remote failed! (%d)\n", result);

		unw_word_t start_ip = 0, offset;
		do
		{
			char buffer[512];
			unw_get_proc_name(&cursor, buffer, sizeof(buffer), &offset);
			procmsg("[STACK_TRACE] (%s+0x%lx)\n", buffer, offset);

			result = unw_step(&cursor);
			if (result < 0)
				procmsg("[UNWIND_ERROR] unw_step failed! (%d)\n", result);
		}
		while (result > 0);
	}

private:
	void *upt_info = nullptr;
	unw_addr_space_t addr_space = 0;
};

/*
// LIBUNWIND DEBUGGING
static void *upt_info = nullptr;
static unw_addr_space_t addr_space = 0;

void setupUnwind()
{
	addr_space = unw_create_addr_space(&_UPT_accessors, 0);
	if (!addr_space)
	{
		procmsg("[UNWIND_ERROR] unw_create_addr_space failed!\n");
	}
}

void setUnwindTarget(pid_t target_pid)
{
	upt_info = _UPT_create(target_pid);
	if (!upt_info)
		procmsg("[UNWIND_ERROR] System is out of memory!\n");
}

void doBacktrace()
{
	unw_cursor_t cursor;
	int result = unw_init_remote(&cursor, addr_space, upt_info);
	if (result < 0)
		procmsg("[UNWIND_ERROR] unw_init_remote failed! (%d)\n", result);

	unw_word_t start_ip = 0, off;
	int n = 0;
	do
	{
		unw_word_t ip, sp;
		if ((result = unw_get_reg(&cursor, UNW_REG_IP, &ip)) < 0 ||
				(result = unw_get_reg(&cursor, UNW_REG_SP, &sp)) < 0)
			procmsg("[UNWIND_ERROR] unw_get_reg failed! (%d)\n", result);

		if (n == 0)
			start_ip = ip;

		// if (print_names)
		char buffer[512];
		unw_get_proc_name(&cursor, buffer, sizeof(buffer), &off);

		// if (verbose)
		// Missed an if (off) bit here as well
		procmsg("[UNWIND] %016lx %-32s (sp=%016lx)\n", (long)ip, buffer, (long)sp);

		unw_proc_info_t proc_info;
		if ((result = unw_get_proc_info(&cursor, &proc_info)) < 0)
			procmsg("[UNWIND_ERROR] unw_get_proc_info failed! (%d)\n", result);
		else // if (verbose)
			procmsg("[UNWIND] \tproc=%016lx-%016lx\n\thandler=%lx lsda=%lx\n",
					(long)proc_info.start_ip, (long)proc_info.end_ip,
					(long)proc_info.handler, (long)proc_info.lsda);

		// if (verbose)
		procmsg("\n");

		result = unw_step(&cursor);
		if (result < 0)
		{
			unw_get_reg(&cursor, UNW_REG_IP, &ip);
			procmsg("[UNWIND_ERROR] unw_step returned %d for ip=%lx (start ip=%lx)\n",
					result, (long)ip, (long)start_ip);
		}

		if (++n > 64)
		{
			// Guard against bad unwind info in old libraries
			procmsg("[UNWIND_ERROR] Too deeply nested - assuming bogus unwind (start ip=%lx)\n", (long)start_ip);
			break;
		}
	}
	while (result > 0);

	if (result < 0)
		procmsg("[UNWIND_ERROR] Unwind failed with result=%d\n", result);

	//_UPT_resume(addr_space, &cursor, upt_info);
}

void stopUnwind()
{
	_UPT_destroy(upt_info);
}
// LIBUNWIND DEBUGGING
*/