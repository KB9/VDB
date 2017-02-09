#pragma once

#include <sys/types.h>
#include <unistd.h>

class VDB
{
public:
	VDB();
	~VDB();

	bool run(const char *executable_name);

private:
	bool runTarget(const char *executable_name);
	bool runDebugger(pid_t child_pid, const char *child_name);
};