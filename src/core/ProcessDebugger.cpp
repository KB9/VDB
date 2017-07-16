#include "ProcessDebugger.hpp"

#include "dwarf/DwarfExprInterpreter.hpp"
#include "ValueDeducer.hpp"
#include "dwarf/DIEVariable.hpp"

#include <cstring>

ProcessDebugger::ProcessDebugger(char *executable_name,
                                 std::shared_ptr<BreakpointTable> breakpoint_table,
                                 std::shared_ptr<DwarfDebug> debug_data)
{
	if (target_name != NULL) delete target_name;
	target_name = new char[strlen(executable_name) + 1];
	strcpy(target_name, executable_name);

	this->breakpoint_table = breakpoint_table;
	this->debug_data = debug_data;

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
				procmsg("[DEBUG] Child process stopped - unknown signal! (%d)\n", last_sig);

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
	auto bph_msg = std::make_unique<BreakpointHitMessage>();
	bph_msg->line_number = breakpoint->line_number;
	bph_msg->file_name = breakpoint->file_name;
	message_queue_out.push(std::move(bph_msg));

	// Wait until an action is taken for this particular breakpoint
	std::unique_lock<std::mutex> lck(mtx);
	while (breakpoint_action == UNDEFINED)
	{
		while (!message_queue_in.empty())
		{
			std::unique_ptr<DebugMessage> msg = message_queue_in.tryPop();
			if (msg == nullptr) continue;

			GetValueMessage *value_msg = dynamic_cast<GetValueMessage *>(msg.get());
			if (value_msg != nullptr)
				deduceValue(value_msg);

			message_queue_out.push(std::move(msg));
		}

		cv.wait(lck);
	}

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
			// TODO: This needs to set a breakpoint on the next line after
			// the line this breakpoint is on.
			breakpoint->stepOver(target_pid);
			break;
		}

		case CONTINUE:
		{
			breakpoint->stepOver(target_pid);
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

void ProcessDebugger::deduceValue(GetValueMessage *value_msg)
{
	// Get the first location expression for the specified variable name
	VariableLocExpr loc_expr;
	bool found = false;
	for (CUHeader header : debug_data->info()->getCUHeaders())
	{
		auto loc_exprs = header.getLocExprsFromVarName<DIEVariable>(value_msg->variable_name);
		if (loc_exprs.size() > 0)
		{
			loc_expr = loc_exprs.at(0);
			found = true;
			break;
		}
	}

	// If no results were found for the specified variable name, return
	if (!found) return;

	// Get the address of the variable in the target process' memory
	DwarfExprInterpreter interpreter(target_pid);
	uint64_t address = interpreter.parse(&loc_expr.frame_base,
	                                     loc_expr.location_op,
	                                     loc_expr.location_param);

	procmsg("[GET_VALUE] Found variable address: 0x%x\n", address);

	// Initialize the value deducer
	ValueDeducer deducer(target_pid, debug_data);
	std::string value;

	procmsg("[GET_VALUE] DWARF variable type offset: 0x%llx\n", loc_expr.type_die_offset);

	// Get the relevant type DIE and deduce the value of the variable
	DebuggingInformationEntry *type_die = debug_data->info()->getDIEByOffset(loc_expr.type_die_offset).get();
	DIEBaseType *base_type_die = dynamic_cast<DIEBaseType *>(type_die);
	if (base_type_die != nullptr)
	{
		// Deduce the value as a base type
		value = deducer.deduce(address, *base_type_die);
		procmsg("[GET_VALUE] Value = %s\n", value.c_str());
        value_msg->value = new char[value.length() + 1];
		strcpy(value_msg->value, ((char *)value.c_str()));
	}

	DIEPointerType *pointer_type_die = dynamic_cast<DIEPointerType *>(type_die);
	if (pointer_type_die != nullptr)
	{
		// Deduce the value as a pointer type
		value = deducer.deduce(address, *pointer_type_die);
		procmsg("[GET_VALUE] Value = %s\n", value.c_str());
		value_msg->value = new char[value.length() + 1];
		strcpy(value_msg->value, ((char *)value.c_str()));
	}
}