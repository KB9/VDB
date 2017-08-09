#include "Unwinder.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

Unwinder::Unwinder(pid_t target_pid)
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

	reset();
}

Unwinder::~Unwinder()
{
	_UPT_destroy(upt_info);
	unw_destroy_addr_space(addr_space);
}

void Unwinder::unwindStep(unsigned int steps)
{
	int result = 0;
	unw_word_t offset;
	unsigned int step_count = 0;
	do
	{
		char buffer[512];
		unw_get_proc_name(&cursor, buffer, sizeof(buffer), &offset);
		procmsg("[STACK_TRACE] (%s+0x%lx)\n", buffer, offset);

		result = unw_step(&cursor);
		if (result < 0)
			procmsg("[UNWIND_ERROR] unw_step failed! (%d)\n", result);

		step_count++;
	}
	while (result > 0 && step_count < steps);
}

unw_word_t Unwinder::getRegisterValue(unw_regnum_t reg_num)
{
	unw_word_t val;
	int result = unw_get_reg(&cursor, reg_num, &val);
	if (result < 0)
		procmsg("[UNWIND_ERROR] unw_get_reg failed! (%d)\n", result);
	return val;
}

void Unwinder::reset()
{
	int result = unw_init_remote(&cursor, addr_space, upt_info);
	if (result < 0)
		procmsg("[UNWIND] unw_init_remote failed! (%d)\n", result);
}