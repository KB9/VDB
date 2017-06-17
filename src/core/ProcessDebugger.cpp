#include "ProcessDebugger.hpp"

ProcessDebugger::ProcessDebugger(char *executable_name,
								 std::shared_ptr<BreakpointTable> breakpoint_table,
								 BreakpointCallback breakpoint_callback)
{
	if (target_name != NULL) delete target_name;
	target_name = new char[strlen(executable_name) + 1];
	strcpy(target_name, executable_name);

	this->breakpoint_table = breakpoint_table;
	this->breakpoint_callback = breakpoint_callback;

	is_debugging = true;
	debug_thread = std::thread(&ProcessDebugger::threadedDebug, this);
}

ProcessDebugger::~ProcessDebugger()
{
	if (target_name != NULL) delete target_name;
	target_name = NULL;

	// Set the variables which will allow the debug thread to terminate
	is_debugging = false;
	breakpoint_action = STEP_OVER;
	cv.notify_all();
	debug_thread.join();
}

void ProcessDebugger::stepOver()
{
	breakpoint_action = STEP_OVER;
	cv.notify_all();
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
	procmsg("Target started. Will run '%s'\n", target_name);

	// Allow tracing of this process
	if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0)
	{
		perror("ptrace");
		return false;
	}

	// Replace this process's image with the given program
	execl(target_name, target_name, 0);

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
				procmsg("[DEBUG] Child process stopped - unknown signal! (%d)\n", last_sig);

				// TODO: Check for other signal types
				// TODO: Continue debugging instead of exiting debug loop
				break;
			}
		}
	}

	return true;
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

	// Notify the listener that a breakpoint has been hit
	breakpoint_callback(this, *breakpoint);

	// Wait until an action is taken for this particular breakpoint
	std::unique_lock<std::mutex> lck(mtx);
	while (breakpoint_action == UNDEFINED) cv.wait(lck);

	// DEBUG: Output the action being performed at this breakpoint
	switch (breakpoint_action)
	{
		case STEP_OVER:
			procmsg("[BREAKPOINT_ACTION] Step over\n");
			break;
		case CONTINUE:
			procmsg("[BREAKPOINT_ACTION] Continue\n");
			break;
		default:
			break;
	}

	// Carry out the action specified for this breakpoint
	switch (breakpoint_action)
	{
		case STEP_OVER:
		{
			breakpoint->stepOver(target_pid);
			break;
		}

		case CONTINUE:
		{
			// TODO: The breakpoint will need to be reset before you can
			// just continue. In this current state, it will crash the
			// program upon CONTINUE-ing.
			break;
		}

		default:
		{
			break;
		}
	}

	// Reset the breakpoint action
	breakpoint_action = UNDEFINED;
}