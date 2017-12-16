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
#include <string>

#include "StepCursor.hpp"
#include "Unwinder.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

// FOWARD DECLARATION [TODO: REMOVE]
unsigned getChildInstructionPointer(pid_t target_pid);

enum BreakpointAction
{
	UNDEFINED,
	STEP_OVER,
	STEP_INTO,
	STEP_OUT,
	CONTINUE
};

class GetStackTraceMessage : public DebugMessage
{
public:
	std::vector<StackEntry> stack;
};

class GetValueMessage : public DebugMessage
{
public:
	std::string variable_name;
	std::string value;
};

class TargetExitMessage : public DebugMessage {};

class BreakpointHitMessage : public DebugMessage
{
public:
	uint64_t line_number;
	std::string file_name;
};

class StepMessage : public DebugMessage
{
public:
	uint64_t line_number;
	std::string file_name;
};

// This class is responsible for forking the process and forming the target and
// debugging process. When running the target process, this class will be
// responsible for informing the breakpoint listener that a breakpoint has been
// hit, allowing them to select an action for that breakpoint.
class ProcessDebugger
{
public:

	ProcessDebugger(const std::string& executable_name,
	                std::shared_ptr<BreakpointTable> breakpoint_table,
	                std::shared_ptr<DwarfDebug> debug_data);
	~ProcessDebugger();

	void continueExecution();
	void stepOver();
	void stepInto();
	void stepOut();

	void enqueue(std::unique_ptr<DebugMessage> msg);
	std::unique_ptr<DebugMessage> tryPoll();

	bool isDebugging();

private:
	std::shared_ptr<DwarfDebug> debug_data = nullptr;

	std::string target_name;
	pid_t target_pid;

	std::shared_ptr<BreakpointTable> breakpoint_table = nullptr;

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
	void processMessageQueue();
	void broadcastBreakpointHit(const std::string &file_name, uint64_t line_number);
	void performStep(StepCursor &cursor, BreakpointAction action);
	void broadcastStep(const std::string &file_name, uint64_t line_number);

	void deduceValue(GetValueMessage *value_msg);
	void getStackTrace(GetStackTraceMessage *stack_msg);
	bool isWithinScope(DebuggingInformationEntry &die);
};