#pragma once

#include <sys/ptrace.h>
#include <stdarg.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <stdio.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "BreakpointTable.hpp"
#include "dwarf/DwarfDebug.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

// FOWARD DECLARATION [TODO: REMOVE]
unsigned getChildInstructionPointer(pid_t target_pid);

enum BreakpointAction
{
	UNDEFINED,
	STEP_OVER,
	CONTINUE
};

class ProcessDebugger;
typedef void (*BreakpointCallback)(ProcessDebugger *debugger, Breakpoint breakpoint);

// This class is responsible for forking the process and forming the target and
// debugging process. When running the target process, this class will be
// responsible for informing the breakpoint listener that a breakpoint has been
// hit, allowing them to select an action for that breakpoint.
class ProcessDebugger
{
public:
	ProcessDebugger(char *executable_name,
					std::shared_ptr<BreakpointTable> breakpoint_table,
					BreakpointCallback breakpoint_callback);
	~ProcessDebugger();

	void stepOver();

private:
	char *target_name = NULL;
	pid_t target_pid;

	std::shared_ptr<BreakpointTable> breakpoint_table = nullptr;
	BreakpointCallback breakpoint_callback;

	bool is_debugging;

	std::thread debug_thread;
	std::mutex mtx;
	std::condition_variable cv;

	BreakpointAction breakpoint_action = UNDEFINED;

	bool threadedDebug();
	bool runTarget();
	bool runDebugger();

	void onBreakpointHit();
};