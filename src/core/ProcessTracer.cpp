#include "ProcessTracer.hpp"

#include <sys/wait.h>

ProcessTracer::ProcessTracer() :
	is_stopped(false),
	is_running(false),
	process(nullptr)
{

}

ProcessTracer::ProcessTracer(ProcessTracer&& other)
{
	process = std::move(other.process);
	is_stopped = other.is_stopped;
	is_running = other.is_running;
}

bool ProcessTracer::start(const std::string& executable_path)
{	
	pid_t child_pid = fork();
	if (child_pid == 0)
	{
		runTarget(executable_path);
		return false;
	}
	else if (child_pid > 0)
	{
		// Get the ID of the child process, and wait for it to stop on its first
		// instruction
		process = std::make_unique<Process>(child_pid);
		::wait(0);
		is_running = true;
		return true;
	}
	else
	{
		perror("fork");
		return false;
	}
}

ProcessTracer::Result<ProcessTracer::Signal> ProcessTracer::continueExec()
{
	if (ptrace(PTRACE_CONT, process->id(), 0, 0) < 0)
	{
		perror("ptrace");
		return make_unexpected("Failed to continue execution");
	}
	return wait();
}

ProcessTracer::Result<ProcessTracer::Signal> ProcessTracer::singleStepExec()
{
	if (ptrace(PTRACE_SINGLESTEP, process->id(), 0, 0))
	{
		perror("ptrace");
		return make_unexpected("Failed to execute single step");
	}
	return wait();
}

ProcessTracer::Result<user_regs_struct> ProcessTracer::getRegisters()
{
	user_regs_struct regs;
	int result = ptrace(PTRACE_GETREGS, process->id(), 0, &regs);
	if (result != -1)
	{
		return regs;
	}
	else
	{
		return make_unexpected("Failed to get register values");
	}
}

ProcessTracer::Result<void> ProcessTracer::setRegisters(const user_regs_struct& regs)
{
	int result = ptrace(PTRACE_SETREGS, process->id(), 0, &regs);
	if (result != -1)
	{
		return {};
	}
	else
	{
		return make_unexpected("Failed to set register values");
	}
}

ProcessTracer::Result<ProcessTracer::Text> ProcessTracer::peekText(Address address)
{
	Text result = ptrace(PTRACE_PEEKTEXT, process->id(), address, 0);
	if (result != -1)
	{
		return result;
	}
	else
	{
		return make_unexpected("Failed to peek text at specified address");
	}
}

ProcessTracer::Result<void> ProcessTracer::pokeText(Address address, Text text)
{
	int result = ptrace(PTRACE_POKETEXT, process->id(), address, text);
	if (result != -1)
	{
		return {};
	}
	else
	{
		return make_unexpected("Failed to poke text at specified address");
	}
}

const std::unique_ptr<Process>& ProcessTracer::tracee() const
{
	return process;
}

bool ProcessTracer::isStopped() const
{
	return is_stopped;
}

bool ProcessTracer::isRunning() const
{
	return is_running;
}

ProcessTracer& ProcessTracer::operator=(ProcessTracer&& other)
{
	process = std::move(other.process);
	is_stopped = other.is_stopped;
	is_running = other.is_running;
}

ProcessTracer::Result<void> ProcessTracer::runTarget(const std::string& executable_path)
{
	// Allow tracing of this process
	if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0)
	{
		perror("ptrace");
		return make_unexpected("Failed to setup target process for tracing");
	}

	// Replace this process's image with the given program
	execl(executable_path.c_str(), executable_path.c_str(), 0);
	return {};
}

ProcessTracer::Result<ProcessTracer::Signal> ProcessTracer::wait()
{
	int wait_status;
	::wait(&wait_status);

	// If the child process exited
	if (WIFEXITED(wait_status))
	{
		is_running = false;
		return WEXITSTATUS(wait_status);
	}
	// If the child process was stopped midway through execution
	else if (WIFSTOPPED(wait_status))
	{
		is_stopped = true;
		return WSTOPSIG(wait_status);
	}
	else
	{
		return make_unexpected("Unknown process status after wait");
	}
}