#include "ValueDeducer.hpp"

#include <sys/ptrace.h>

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

ValueDeducer::ValueDeducer(pid_t target_pid)
{
	this->target_pid = target_pid;
}

std::string ValueDeducer::deduce(uint64_t address, const DIEBaseType &base_die)
{
	// Get the data at the specified target process address
	uint64_t data = ptrace(PTRACE_PEEKDATA, target_pid, address, 0);

	// Use the encoding and byte size to determine the data's type
	switch (base_die.getEncoding())
	{
	//case DW_ATE_address:

	case DW_ATE_boolean:
	{
		bool value = (bool)data;
		return std::to_string(value);
	}

	//case DW_ATE_complex_float:

	case DW_ATE_float:
	{
		float value = (float)data;
		return std::to_string(value);
	}

	//case DW_ATE_imaginary_float:

	case DW_ATE_signed:
	{
		if (base_die.getByteSize() == 1)
		{
			char value = (char)data;
			return std::to_string(value);
		}
		else if (base_die.getByteSize() == 2)
		{
			short int value = (short int)data;
			return std::to_string(value);
		}
		else if (base_die.getByteSize() == 4)
		{
			int value = (int)data;
			return std::to_string(value);
		}
		else if (base_die.getByteSize() == 8)
		{
			long int value = (long int)data;
			return std::to_string(value);
		}
	}

	case DW_ATE_signed_char:
	{
		char value = (char)data;
		return std::to_string(value);
	}

	case DW_ATE_unsigned:
	{
		if (base_die.getByteSize() == 1)
		{
			unsigned char value = (unsigned char)data;
			return std::to_string(value);
		}
		else if (base_die.getByteSize() == 2)
		{
			short unsigned int value = (short unsigned int)data;
			return std::to_string(value);
		}
		else if (base_die.getByteSize() == 4)
		{
			unsigned int value = (unsigned int)data;
			return std::to_string(value);
		}
		else if (base_die.getByteSize() == 8)
		{
			long unsigned int value = (long unsigned int)data;
			return std::to_string(value);
		}
	}

	case DW_ATE_unsigned_char:
	{
		unsigned char value = (unsigned char)data;
		return std::to_string(value);
	}

	//case DW_ATE_packed_decimal:
	
	//case DW_ATE_numeric_string:
	
	//case DW_ATE_edited:
	
	//case DW_ATE_signed_fixed:
	
	//case DW_ATE_unsigned_fixed:
	
	//case DW_ATE_decimal_float:
	}

	return "";
}

std::string ValueDeducer::deduce(uint64_t address, const DIEPointerType &pointer_die)
{
	return "pointer";
}