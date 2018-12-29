#include "ProcessDebugger.hpp"

#include "Unwinder.hpp"

#include <cstring>
#include <cassert>

void printProcessSignal(int signal)
{
	switch (signal)
	{
		case SIGABRT: procmsg("Process abort (SIGABRT)\n"); break;
		case SIGALRM: procmsg("Alarm clock (SIGALRM)\n"); break;
		case SIGFPE: procmsg("Erroneous arithmetic operation (SIGFPE)\n"); break;
		case SIGHUP: procmsg("Hangup (SIGHUP)\n"); break;
		case SIGILL: procmsg("Illegal instruction (SIGILL)\n"); break;
		case SIGINT: procmsg("Terminal interrupt signal (SIGINT)\n"); break;
		case SIGKILL: procmsg("Kill (SIGKILL)\n"); break;
		case SIGPIPE: procmsg("Write on a pipe with no one to read it (SIGPIPE)\n"); break;
		case SIGQUIT: procmsg("Terminal quit signal (SIGQUIT)\n"); break;
		case SIGSEGV: procmsg("Invalid memory reference (SIGSEGV)\n"); break;
		case SIGTERM: procmsg("Termination signal (SIGTERM)\n"); break;
		case SIGUSR1: procmsg("User-defined signal 1 (SIGUSR1)\n"); break;
		case SIGUSR2: procmsg("User-defined signal 2 (SIGUSR2)\n"); break;
		case SIGCHLD: procmsg("Child process terminated or stopped (SIGCHLD)\n"); break;
		case SIGCONT: procmsg("Continue executing, if stopped (SIGCONT)\n"); break;
		case SIGSTOP: procmsg("Stop executing (SIGSTOP)\n"); break;
		case SIGTSTP: procmsg("Terminal stop signal (SIGTSTP)\n"); break;
		case SIGTTIN: procmsg("Background process attempting read (SIGTTIN)\n"); break;
		case SIGTTOU: procmsg("Background process attempting write (SIGTTOU)\n"); break;
		case SIGBUS: procmsg("Access to an undefined portion of a memory object (SIGBUS)\n"); break;
		case SIGPOLL: procmsg("Pollable event (SIGPOLL)\n"); break;
		case SIGPROF: procmsg("Profiling timer expired (SIGPROF)\n"); break;
		case SIGSYS: procmsg("Bad system call (SIGSYS)\n"); break;
		case SIGURG: procmsg("High bandwidth data is available at a socket (SIGURG)\n"); break;
		case SIGVTALRM: procmsg("Virtual timer expired (SIGVTALRM)\n"); break;
		case SIGXCPU: procmsg("CPU time limit exceeded (SIGXCPU)\n"); break;
		case SIGXFSZ: procmsg("File size limit exceeded (SIGXFSZ)\n"); break;
		default: procmsg("UNKNOWN\n");
	}
}

ProcessDebugger::ProcessDebugger(const std::string& executable_name,
                                 std::vector<BreakpointLine> breakpoint_lines,
                                 std::shared_ptr<DebugInfo> debug_info) :
	debug_info(debug_info),
	target_name(executable_name),
	breakpoint_lines(breakpoint_lines),
	elf_file(std::make_unique<ELFFile>(executable_name))
{
	is_debugging = true;
	debug_thread = std::thread(&ProcessDebugger::runDebugger, this);
}

ProcessDebugger::~ProcessDebugger()
{
	// Set the variables which will allow the debug thread to terminate
	is_debugging = false;
	breakpoint_action = CONTINUE;
	cv.notify_all();
	debug_thread.join();
}

void ProcessDebugger::enqueue(std::unique_ptr<DebugMessage> msg)
{
	message_queue_in.push(std::move(msg));
	cv.notify_all();
}

std::unique_ptr<DebugMessage> ProcessDebugger::tryPoll()
{
	return std::move(message_queue_out.tryPop());
}

void ProcessDebugger::stepOver()
{
	breakpoint_action = STEP_OVER;
	cv.notify_all();
}

void ProcessDebugger::stepInto()
{
	breakpoint_action = STEP_INTO;
	cv.notify_all();
}

void ProcessDebugger::stepOut()
{
	breakpoint_action = STEP_OUT;
	cv.notify_all();
}

void ProcessDebugger::continueExecution()
{
	breakpoint_action = CONTINUE;
	cv.notify_all();
}

bool ProcessDebugger::isDebugging()
{
	return is_debugging;
}

// ===== EVERYTHING BELOW THIS LINE IS RUN IN THE PRIVATE DEBUGGER THREAD =====

bool ProcessDebugger::runDebugger()
{
	bool is_tracee_started = tracer.start(target_name);
	if (!is_tracee_started)
		return false;

	memory_mappings = std::make_unique<ProcessMemoryMappings>(tracer.traceePID());
	createBreakpoints();
	createEntryBreakpoint();

	breakpoint_table->enableBreakpoints(tracer);

	while (is_debugging)
	{
		// Resume execution
		auto expected_signal = tracer.continueExec();
		if (!expected_signal.has_value())
			return false;
		int signal = expected_signal.value();

		// If the child process exited
		if (!tracer.isRunning())
		{
			is_debugging = false;
			break;
		}
		// If the child process was stopped midway through execution
		else if (tracer.isStopped())
		{
			// If a breakpoint was hit
			if (signal == SIGTRAP)
			{
				procmsg("[DEBUG] Breakpoint hit!\n");

				// Find the correct breakpoint and notify the listener of it
				onBreakpointHit();

				// Continue execution of child process
				continue;
			}
			else
			{
				procmsg("[DEBUG] Child process stopped: ");
				printProcessSignal(signal);

				// TODO: Check for other signal types
				// TODO: Continue debugging instead of exiting debug loop
				is_debugging = false;
				break;
			}
		}
	}

	// Notify the frontend that the target process has exited
	message_queue_out.push(std::make_unique<TargetExitMessage>());

	return true;
}

void ProcessDebugger::createBreakpoints()
{
	breakpoint_table = std::make_unique<BreakpointTable>();

	uint64_t start_address_offset = 0;
	if (elf_file->hasPositionIndependentCode())
	{
		start_address_offset = memory_mappings->loadAddress();
	}

	for (const auto& bp_line : breakpoint_lines)
	{
		for (const DebugInfo::SourceLine &line : debug_info->getSourceFileLines(bp_line.file_name))
		{
			bool is_match = line.file_name == bp_line.file_name && line.number == bp_line.line_number;
			bool is_not_breakpoint = !breakpoint_table->isBreakpoint(line.address);
			if (is_match && is_not_breakpoint)
			{
				uint64_t bp_address = start_address_offset + line.address;
				breakpoint_table->addBreakpoint(bp_address);
				breakpoint_lines_by_address.emplace(bp_address, bp_line);

				// TODO: By breaking here, you are assuming the first address found is
				// the lowest address of this source line's assembly. This may be
				// changed in future versions.
				break;
			}
		}
	}
}

void ProcessDebugger::createEntryBreakpoint()
{
	uint64_t entry_address = elf_file->entryPoint();
	if (elf_file->hasPositionIndependentCode())
	{
		entry_address += memory_mappings->loadAddress();
	}

	entry_breakpoint = std::make_unique<Breakpoint>(entry_address);
	entry_breakpoint->enable(tracer);
}

void ProcessDebugger::processMessageQueue()
{
	while (!message_queue_in.empty())
	{
		std::unique_ptr<DebugMessage> msg = message_queue_in.tryPop();
		if (msg == nullptr) continue;

		GetValueMessage *value_msg = dynamic_cast<GetValueMessage *>(msg.get());
		if (value_msg != nullptr)
			deduceValue(value_msg);

		GetStackTraceMessage *stack_msg = dynamic_cast<GetStackTraceMessage *>(msg.get());
		if (stack_msg != nullptr)
			getStackTrace(stack_msg);

		message_queue_out.push(std::move(msg));
	}
}

void ProcessDebugger::broadcastBreakpointHit(const std::string &file_name,
                                             uint64_t line_number)
{
	// Notify the frontend that a breakpoint has been hit
	auto bph_msg = std::make_unique<BreakpointHitMessage>();
	bph_msg->line_number = line_number;
	bph_msg->file_name = file_name;
	message_queue_out.push(std::move(bph_msg));
}

void ProcessDebugger::broadcastStep(const std::string &file_name,
                                    uint64_t line_number)
{
	// Notify the frontend that a program step has been performed
	auto step_msg = std::make_unique<StepMessage>();
	step_msg->line_number = line_number;
	step_msg->file_name = file_name;
	message_queue_out.push(std::move(step_msg));
}

void ProcessDebugger::performStep(StepCursor &cursor, BreakpointAction action)
{
	// Perform the step action
	switch (action)
	{
		case STEP_OVER:
		{
			cursor.stepOver(tracer);
			break;
		}
		case STEP_INTO:
		{
			cursor.stepInto(tracer);
			break;
		}
		case STEP_OUT:
		{
			cursor.stepOut(tracer);
			break;
		}
		default:
			break;
	}

	// Report to the frontend message receiver
	broadcastStep(cursor.getCurrentSourceFile(tracer),
	              cursor.getCurrentLineNumber(tracer));
}

void ProcessDebugger::onBreakpointHit()
{
	procmsg("[DEBUG] Getting breakpoint at address: 0x%lx\n", getAbsoluteIP(tracer) - 1);

	// TESTING
	auto libraries = so_observer.getLoadedObjects(tracer, *elf_file, *memory_mappings);
	for (const auto& name : libraries)
		procmsg("[SHARED_OBJECT] %s\n", name.c_str());

	uint64_t breakpoint_address = getAbsoluteIP(tracer) - 1;
	if (entry_breakpoint->addr == breakpoint_address)
	{
		onEntryBreakpointHit();
	}
	else if (so_observer.getRendezvousBreakpoint()->addr == breakpoint_address)
	{
		onRendezvousBreakpointHit();
	}
	else
	{
		onUserBreakpointHit();
	}
}

void ProcessDebugger::onEntryBreakpointHit()
{
	so_observer.setRendezvousBreakpoint(tracer, *elf_file, *memory_mappings);

	procmsg("[ENTRY_POINT] Stepping over entry breakpoint!\n");
	entry_breakpoint->stepOver(tracer);
}

void ProcessDebugger::onRendezvousBreakpointHit()
{
	procmsg("[ENTRY_POINT] Stepping over rendezvous breakpoint!\n");
	auto& rendezvous_breakpoint = so_observer.getRendezvousBreakpoint();
	rendezvous_breakpoint->stepOver(tracer);
}

void ProcessDebugger::onUserBreakpointHit()
{
	// Notify the frontend that a breakpoint has been hit
	uint64_t breakpoint_address = getAbsoluteIP(tracer) - 1;
	BreakpointLine line = breakpoint_lines_by_address.at(breakpoint_address);
	broadcastBreakpointHit(line.file_name, line.line_number);

	// Create the step cursor at the address the program is currently stopped at
	uint64_t load_address_offset = 0;
	if (elf_file->hasPositionIndependentCode())
	{
		load_address_offset = memory_mappings->loadAddress();
	}
	StepCursor step_cursor(debug_info, breakpoint_table, load_address_offset);

	// Wait until an action is taken for this particular breakpoint
	std::unique_lock<std::mutex> lck(mtx);
	while (breakpoint_action != CONTINUE)
	{
		// Process the incoming message queue
		processMessageQueue();

		// Perform any stepping actions required
		if (breakpoint_action == STEP_OVER || breakpoint_action == STEP_INTO ||
		    breakpoint_action == STEP_OUT)
		{
			performStep(step_cursor, breakpoint_action);
		}

		// Reset the breakpoint action
		breakpoint_action = UNDEFINED;

		// Wait until notified
		cv.wait(lck);
	}

	// If the step cursor is currently stopped on a user breakpoint, step over
	// it first to execute the instruction before continuing
	uint64_t potential_user_bp_address = getAbsoluteIP(tracer) - 1;
	if (breakpoint_table->isBreakpoint(potential_user_bp_address))
	{
		Breakpoint &user_bp = breakpoint_table->getBreakpoint(potential_user_bp_address);
		user_bp.stepOver(tracer);
	}

	// Reset the breakpoint action
	breakpoint_action = UNDEFINED;
}

void ProcessDebugger::deduceValue(GetValueMessage *value_msg)
{
	DebugInfo::Variable var = debug_info->getVariable(value_msg->variable_name,
	                                                  tracer.traceePID());
	value_msg->value = var.value;
}

void ProcessDebugger::getStackTrace(GetStackTraceMessage *stack_msg)
{
	Unwinder unwinder(tracer.traceePID());
	stack_msg->stack = unwinder.traceStack();
	unwinder.reset();
}

uint64_t ProcessDebugger::getAbsoluteIP(ProcessTracer& tracer)
{
	// Get the breakpoint that was hit
	auto expected_regs = tracer.getRegisters();
	assert(expected_regs.has_value());
	user_regs_struct regs = expected_regs.value();
	return regs.rip;
}