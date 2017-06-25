#pragma once

#include <sys/ptrace.h>
#include <stdarg.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <stdio.h>

#include <thread>
#include <mutex>
#include <condition_variable>

#include "BreakpointTable.hpp"
#include "dwarf/DwarfDebug.hpp"

#include "ThreadSafeQueue.hpp"

#include <memory>

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

class GetValueMessage : public DebugMessage
{
public:
	char *variable_name;
	char *value;
};

// This class is responsible for forking the process and forming the target and
// debugging process. When running the target process, this class will be
// responsible for informing the breakpoint listener that a breakpoint has been
// hit, allowing them to select an action for that breakpoint.
class ProcessDebugger
{
public:

	ProcessDebugger(char *executable_name,
					std::shared_ptr<BreakpointTable> breakpoint_table,
					BreakpointCallback breakpoint_callback,
					std::shared_ptr<DwarfDebug> debug_data);
	~ProcessDebugger();

	void stepOver();

	void enqueue(std::unique_ptr<DebugMessage> msg);
	std::unique_ptr<DebugMessage> tryPoll();

private:
	std::shared_ptr<DwarfDebug> debug_data = nullptr;

	char *target_name = NULL;
	pid_t target_pid;

	std::shared_ptr<BreakpointTable> breakpoint_table = nullptr;
	BreakpointCallback breakpoint_callback;

	bool is_debugging;

	ThreadSafeQueue<DebugMessage> message_queue_in;
	ThreadSafeQueue<DebugMessage> message_queue_out;

	std::thread debug_thread;
	std::mutex mtx;
	std::condition_variable cv;

	BreakpointAction breakpoint_action = UNDEFINED;

	// Variable related to retrieving the values of the target's variables
	bool deduction_enabled = false;
	char **deduced_value = nullptr;
	VariableLocExpr loc_expr;
	DebuggingInformationEntry *type_die = nullptr;

	bool threadedDebug();
	bool runTarget();
	bool runDebugger();

	void onBreakpointHit();

	void deduceValue(GetValueMessage *value_msg);
};