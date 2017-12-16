#include "ProcessDebugger.hpp"

#include "dwarf/DwarfExprInterpreter.hpp"
#include "ValueDeducer.hpp"
#include "dwarf/DIEVariable.hpp"

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
                                 std::shared_ptr<BreakpointTable> breakpoint_table,
                                 std::shared_ptr<DwarfDebug> debug_data) :
	debug_data(debug_data),
	target_name(executable_name),
	breakpoint_table(breakpoint_table)
{
	is_debugging = true;
	debug_thread = std::thread(&ProcessDebugger::threadedDebug, this);
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

bool ProcessDebugger::threadedDebug()
{
	pid_t child_pid = fork();
	if (child_pid == 0)
	{
		runTarget();
	}
	else if (child_pid > 0)
	{
		target_pid = child_pid;
		runDebugger();
	}
	else
	{
		perror("fork");
		return false;
	}

	return true;
}

bool ProcessDebugger::runTarget()
{
	procmsg("Target started. Will run '%s'\n", target_name.c_str());

	// Allow tracing of this process
	if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0)
	{
		perror("ptrace");
		return false;
	}

	// Replace this process's image with the given program
	execl(target_name.c_str(), target_name.c_str(), 0);

	return false;
}

bool ProcessDebugger::runDebugger()
{
	// Wait for child to stop on its first instruction
	wait(0);
	procmsg("Entry point. EIP = 0x%08x\n", getChildInstructionPointer(target_pid));

	breakpoint_table->enableBreakpoints(target_pid);

	int wait_status;

	while (is_debugging)
	{
		// Resume execution
		if (ptrace(PTRACE_CONT, target_pid, 0, 0) < 0)
		{
			perror("ptrace");
			return false;
		}
		wait(&wait_status);

		// If the child process exited
		if (WIFEXITED(wait_status))
		{
			// Stop debugging
			is_debugging = false;
			break;
		}
		// If the child process was stopped midway through execution
		else if (WIFSTOPPED(wait_status))
		{
			int last_sig = WSTOPSIG(wait_status);

			// If a breakpoint was hit
			if (last_sig == SIGTRAP)
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
				printProcessSignal(last_sig);

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
			cursor.stepOver(target_pid);
			break;
		}
		case STEP_INTO:
		{
			cursor.stepInto(target_pid);
			break;
		}
		case STEP_OUT:
		{
			cursor.stepOut(target_pid);
			break;
		}
		default:
			break;
	}

	// Report to the frontend message receiver
	broadcastStep(cursor.getCurrentSourceFile(), cursor.getCurrentLineNumber());
}

void ProcessDebugger::onBreakpointHit()
{
	// Get the breakpoint that was hit
	user_regs_struct regs;
	ptrace(PTRACE_GETREGS, target_pid, 0, &regs);
	procmsg("[DEBUG] Getting breakpoint at address: 0x%08x\n", regs.rip);
	uint64_t breakpoint_address = regs.rip - 1;
	std::unique_ptr<Breakpoint> breakpoint =
		breakpoint_table->getBreakpoint(breakpoint_address);

	// If it is not a valid breakpoint, return
	if (!breakpoint) return;

	// DEBUG: Notify that the debug thread is waiting for a breakpoint action
	procmsg("[BREAKPOINT_ACTION] Waiting for breakpoint action...\n");

	// Notify the frontend that a breakpoint has been hit
	broadcastBreakpointHit(breakpoint->file_name, breakpoint->line_number);

	// Create the step cursor at the address the program is currently stopped at
	StepCursor step_cursor(breakpoint_address, debug_data, breakpoint_table);

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
	std::unique_ptr<Breakpoint> user_bp =
		breakpoint_table->getBreakpoint(step_cursor.getCurrentAddress());
	if (user_bp != nullptr)
		user_bp->stepOver(target_pid);

	// Reset the breakpoint action
	breakpoint_action = UNDEFINED;
}

void ProcessDebugger::deduceValue(GetValueMessage *value_msg)
{
	// Get the first location expression for the specified variable name
	VariableLocExpr loc_expr;
	bool found = false;
	for (CUHeader header : debug_data->info()->getCUHeaders())
	{
		auto loc_exprs = header.getLocExprsFromVarName<DIEVariable>(value_msg->variable_name.c_str());
		if (loc_exprs.size() > 0)
		{
			// TODO: What happens if there's more than one location expression?
			loc_expr = loc_exprs.at(0);
			found = true;
			break;
		}
	}

	// If no results were found for the specified variable name, return
	if (!found)
	{
		value_msg->value = "Variable not found";
		return;
	}

	// TESTING: Check if the variable is within scope
	if (!isWithinScope(*(loc_expr.die.get())))
	{
		value_msg->value = "Variable not in scope";
		return;
	}

	// Get the address of the variable in the target process' memory
	DwarfExprInterpreter interpreter(target_pid);
	uint64_t address = interpreter.parse(&loc_expr.frame_base,
	                                     loc_expr.location_op,
	                                     loc_expr.location_param);
	if (address > 0)
	{
		procmsg("[GET_VALUE] Found variable address: 0x%x\n", address);

		// Initialize the value deducer
		ValueDeducer deducer(target_pid, debug_data);
		std::string value;

		procmsg("[GET_VALUE] DWARF variable type offset: 0x%llx\n", loc_expr.type_die_offset);

		// Get the relevant type DIE and deduce the value of the variable
		DebuggingInformationEntry *type_die = debug_data->info()->getDIEByOffset(loc_expr.type_die_offset).get();
		value_msg->value = deducer.deduce(address, *type_die);
	}
	else
	{
		value_msg->value = "Variable not locatable";
	}
}

void ProcessDebugger::getStackTrace(GetStackTraceMessage *stack_msg)
{
	Unwinder unwinder(target_pid);
	stack_msg->stack = unwinder.traceStack();
	unwinder.reset();
}

// Backtracks up the DIE tree from the specified DIE until the first
// subprogram DIE is found, then check if the IP falls within this
// subprogram's address range.
// Also checks for global variables by looking at the base of the DIE tree.
bool ProcessDebugger::isWithinScope(DebuggingInformationEntry &die)
{
	user_regs_struct regs;
	ptrace(PTRACE_GETREGS, target_pid, 0, &regs);

	// Check if the variable is in global scope first by checking if the parent
	// DIE is the root DIE.
	DIECompileUnit *compile_unit = dynamic_cast<DIECompileUnit *>(die.getParent());
	if (compile_unit != nullptr)
		return true;

	// Navigate up the tree until the first subprogram is found
	DebuggingInformationEntry *temp_die = &die;
	while (temp_die != nullptr)
	{
		DIESubprogram *subprogram = dynamic_cast<DIESubprogram *>(temp_die);
		if (subprogram != nullptr)
		{
			// If the IP register falls into the subprogram's address range
			if (regs.rip >= subprogram->lowpc &&
			    regs.rip < (subprogram->lowpc + subprogram->highpc))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			temp_die = temp_die->getParent();
		}
	}
	return false;
}