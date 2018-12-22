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
#include "DebugInfo.hpp"

#include "ThreadSafeQueue.hpp"

#include <memory>
#include <string>
#include <map>

#include "ProcessTracer.hpp"
#include "StepCursor.hpp"
#include "Unwinder.hpp"
#include "ELFFile.hpp"
#include "ProcessMemoryMappings.hpp"
#include "SharedObjectObserver.hpp"

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

struct BreakpointLine
{
	uint64_t line_number;
	std::string file_name;

	bool operator==(const BreakpointLine& other) const
	{
		return line_number == other.line_number && file_name == other.file_name;
	}
};

// This class is responsible for forking the process and forming the target and
// debugging process. When running the target process, this class will be
// responsible for informing the breakpoint listener that a breakpoint has been
// hit, allowing them to select an action for that breakpoint.
class ProcessDebugger
{
public:

	ProcessDebugger(const std::string& executable_name,
	                std::vector<BreakpointLine> breakpoint_lines,
	                std::shared_ptr<DebugInfo> debug_info);
	~ProcessDebugger();

	void continueExecution();
	void stepOver();
	void stepInto();
	void stepOut();

	void enqueue(std::unique_ptr<DebugMessage> msg);
	std::unique_ptr<DebugMessage> tryPoll();

	bool isDebugging();

private:
	std::shared_ptr<DebugInfo> debug_info = nullptr;

	std::string target_name;
	ProcessTracer tracer;

	std::vector<BreakpointLine> breakpoint_lines;
	std::map<uint64_t, BreakpointLine> breakpoint_lines_by_address;
	std::shared_ptr<BreakpointTable> breakpoint_table = nullptr;

	bool is_debugging;

	ThreadSafeQueue<DebugMessage> message_queue_in;
	ThreadSafeQueue<DebugMessage> message_queue_out;

	std::thread debug_thread;
	std::mutex mtx;
	std::condition_variable cv;

	BreakpointAction breakpoint_action = UNDEFINED;

	std::unique_ptr<ELFFile> elf_file = nullptr;
	std::unique_ptr<ProcessMemoryMappings> memory_mappings = nullptr;

	SharedObjectObserver so_observer;

	bool runDebugger();

	void createBreakpoints();

	void onBreakpointHit();
	void onUserBreakpointHit();
	void processMessageQueue();
	void broadcastBreakpointHit(const std::string &file_name, uint64_t line_number);
	void performStep(StepCursor &cursor, BreakpointAction action);
	void broadcastStep(const std::string &file_name, uint64_t line_number);

	void deduceValue(GetValueMessage *value_msg);
	void getStackTrace(GetStackTraceMessage *stack_msg);

	uint64_t getAbsoluteIP(ProcessTracer& tracer);
};