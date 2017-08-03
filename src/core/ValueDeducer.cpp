#include "ValueDeducer.hpp"

#include <sys/ptrace.h>

#include <cmath>

#include "dwarf/DIESubrangeType.hpp"

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

ValueDeducer::ValueDeducer(pid_t target_pid, std::shared_ptr<DwarfDebug> debug_data)
{
	this->target_pid = target_pid;
	this->debug_data = debug_data;
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
		// Decoding according to IEEE-754 floating-point standard:
		// - The sign is a single bit.
		// - The exponent is stored as an 8-bit integer.
		// - The mantissa is stored as a 23-bit integer.
		uint64_t sign_bit = (data >> 31) & 1;
		uint64_t exponent_bits = (data >> 23) & 0xFF;
		uint64_t mantissa_bits = data & 0x7FFFFF;

		// Calculate the actual exponent value, accounting for exponent underflow.
		// Exponent underflow occurs when the exponent bits are equal to 0, in
		// which case the exponent is set to -126 and the leading mantissa
		// bit (1) is not used.
		int64_t exponent = (exponent_bits > 0 ? exponent_bits - 127 : -126);

		// Calculate the actual mantissa value (accounting for exponent underflow).
		// There is an invsibile leading bit, and then the mantissa bits are used
		// by multiplying the first bit by 1/2, then the second by 1/4, third by
		// an 1/8, etc.
		float mantissa = (exponent_bits > 0 ? 1.0f : 0.0f);
		for (int i = 0; i < 23; i++)
		{
			uint64_t mantissa_bit = (mantissa_bits >> (22 - i)) & 1;
			mantissa += ((float)mantissa_bit * std::pow(2.0f, (float)-(i+1)));
		}

		// Calculate the floating point value stored in memory.
		// Note that this may not be the same as the value declared, as the
		// conversion for storage in memory introduces a degree of inaccuracy..
		float value = std::pow(-1.0f, (float)sign_bit) * mantissa * std::pow(2.0f, (float)exponent);

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
		return std::string(1, value);
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
	std::shared_ptr<DebuggingInformationEntry> die = debug_data->info()->getDIEByOffset(pointer_die.type_offset);
	if (die == nullptr) return "Error retrieving type pointed to";

	uint64_t new_address = ptrace(PTRACE_PEEKDATA, target_pid, address, 0);

	DIEPointerType *recurring_pointer_die = dynamic_cast<DIEPointerType *>(die.get());
	if (recurring_pointer_die != nullptr)
	{
		return deduce(new_address, *recurring_pointer_die);
	}

	DIEBaseType *base_die = dynamic_cast<DIEBaseType *>(die.get());
	if (base_die != nullptr)
	{
		return deduce(new_address, *base_die);
	}

	return "Error deducing type pointed to";
}

std::string ValueDeducer::deduce(uint64_t address, DIEArrayType &array_die)
{
	// Get the type of the array
	std::shared_ptr<DebuggingInformationEntry> die = debug_data->info()->getDIEByOffset(array_die.getTypeOffset());
	if (die == nullptr) return "Error getting array type";
	DIEBaseType *array_base_type_die = dynamic_cast<DIEBaseType *>(die.get());

	// Find the subrange child DIE to determine the upper bound of the array
	uint64_t upper_bound = 0;
	auto child_ptrs = array_die.getChildren();
	for (auto child_ptr : child_ptrs)
	{
		if (child_ptr->getTagName() == "DW_TAG_subrange_type")
		{
			auto child = dynamic_cast<DIESubrangeType *>(child_ptr.get());
			upper_bound = child->getUpperBound();
			break;
		}
	}

	// Deduce the values of the array's elements
	std::string array_string = "{";
	uint64_t array_type_size = array_base_type_die->getByteSize();
	uint64_t array_size = array_type_size * (upper_bound + 1);
	for (uint64_t i = 0; i < array_size; i += array_type_size)
	{
		// Deduce the array values and add them to the string
		array_string += deduce(address + i, *array_base_type_die);
		if (i < (array_size - array_type_size)) array_string += ",";
	}
	array_string += "}";
	return array_string;
}