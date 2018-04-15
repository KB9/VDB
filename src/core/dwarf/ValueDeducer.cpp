#include "ValueDeducer.hpp"

#include <sys/ptrace.h>

#include <cmath>
#include <cassert>

// FOWARD DECLARATION [TODO: REMOVE]
void procmsg(const char* format, ...);

ValueDeducer::ValueDeducer(pid_t target_pid, std::shared_ptr<DwarfDebug> debug_data)
{
	this->target_pid = target_pid;
	this->debug_data = debug_data;
}

std::string ValueDeducer::deduce(uint64_t address, DIE &type_die)
{
	if (type_die.getTagName() == "DW_TAG_base_type")
	{
		return deduceBase(address, type_die);
	}
	else if (type_die.getTagName() == "DW_TAG_pointer_type")
	{
		return deducePointer(address, type_die);
	}
	else if (type_die.getTagName() == "DW_TAG_reference_type")
	{
		return deduceReference(address, type_die);
	}
	else if (type_die.getTagName() == "DW_TAG_array_type")
	{
		return deduceArray(address, type_die);
	}
	else if (type_die.getTagName() == "DW_TAG_structure_type")
	{
		return deduceStructure(address, type_die);
	}
	else if (type_die.getTagName() == "DW_TAG_class_type")
	{
		return deduceClass(address, type_die);
	}
	else if (type_die.getTagName() == "DW_TAG_const_type")
	{
		return deduceConst(address, type_die);
	}
	else
	{
		return "Type cannot be deduced";
	}
}

std::string ValueDeducer::deduceBase(uint64_t address, const DIE &base_die)
{
	assert(base_die.getTagName() == "DW_TAG_base_type");

	// Get the data at the specified target process address
	uint64_t data = ptrace(PTRACE_PEEKDATA, target_pid, address, 0);

	// Use the encoding and byte size to determine the data's type
	Attribute encoding = base_die.getAttributeByCode(DW_AT_encoding);
	Attribute byte_size = base_die.getAttributeByCode(DW_AT_byte_size);
	switch (encoding.getUnsigned())
	{
	//case DW_ATE_address:

	case DW_ATE_boolean:
	{
		bool value = (bool)data;
		return value ? "true" : "false";
	}

	//case DW_ATE_complex_float:

	case DW_ATE_float:
	{
		if (byte_size.getUnsigned() == 4)
		{
			float value = *((float *)&data);
			return std::to_string(value);
		}
		else if (byte_size.getUnsigned() == 8)
		{
			double value = *((double *)&data);
			return std::to_string(value);
		}
	}

	//case DW_ATE_imaginary_float:

	case DW_ATE_signed:
	{
		if (byte_size.getUnsigned() == 1)
		{
			char value = (char)data;
			return std::to_string(value);
		}
		else if (byte_size.getUnsigned() == 2)
		{
			short int value = (short int)data;
			return std::to_string(value);
		}
		else if (byte_size.getUnsigned() == 4)
		{
			int value = (int)data;
			return std::to_string(value);
		}
		else if (byte_size.getUnsigned() == 8)
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
		if (byte_size.getUnsigned() == 1)
		{
			unsigned char value = (unsigned char)data;
			return std::to_string(value);
		}
		else if (byte_size.getUnsigned() == 2)
		{
			short unsigned int value = (short unsigned int)data;
			return std::to_string(value);
		}
		else if (byte_size.getUnsigned() == 4)
		{
			unsigned int value = (unsigned int)data;
			return std::to_string(value);
		}
		else if (byte_size.getUnsigned() == 8)
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

	return "<unknown>";
}

std::string ValueDeducer::deducePointer(uint64_t address, const DIE &pointer_die)
{
	Attribute type = pointer_die.getAttributeByCode(DW_AT_type);
	DIE type_die = *(debug_data->info()->getDIEByOffset(type.getOffset()));

	uint64_t new_address = ptrace(PTRACE_PEEKDATA, target_pid, address, 0);
	return deduce(new_address, type_die);
}

std::string ValueDeducer::deduceReference(uint64_t address, const DIE &ref_die)
{
	Attribute type = ref_die.getAttributeByCode(DW_AT_type);
	DIE type_die = *(debug_data->info()->getDIEByOffset(type.getOffset()));

	uint64_t new_address = ptrace(PTRACE_PEEKDATA, target_pid, address, 0);
	return deduce(new_address, type_die);
}

std::string ValueDeducer::deduceArray(uint64_t address, DIE &array_die)
{
	assert(array_die.getTagName() == "DW_TAG_array_type");

	// Get the type of the array
	Attribute type = array_die.getAttributeByCode(DW_AT_type);
	DIE type_die = *(debug_data->info()->getDIEByOffset(type.getOffset()));

	// Find the subrange child DIE to determine the upper bound of the array
	uint64_t array_length = 0;
	bool found_array_length = false;
	std::vector<DIE> children = array_die.getChildren();
	for (auto &child : children)
	{
		if (child.getTagName() == "DW_TAG_subrange_type")
		{
			Attribute upper_bound = child.getAttributeByCode(DW_AT_upper_bound);
			array_length = upper_bound.getUnsigned();
			found_array_length = true;
		}
	}
	if (!found_array_length) return "Could not determine array length";

	// Deduce the array's contents
	std::string values = "{";
	Attribute type_attr_byte_size = type_die.getAttributeByCode(DW_AT_byte_size);
	uint64_t type_byte_size = type_attr_byte_size.getUnsigned();
	uint64_t array_byte_size = type_byte_size * (array_length + 1);
	for (uint64_t i = 0; i < array_byte_size; i += type_byte_size)
	{
		// Add a comma before adding the next value
		if (i > 0) values += ", ";

		values += deduce(address + i, type_die);
	}
	values += "}";

	return values;
}

std::string ValueDeducer::deduceStructure(uint64_t address, DIE &struct_die)
{	
	std::string values = "{";

	uint64_t counter = 0;
	std::vector<DIE> children = struct_die.getChildren();
	for (auto &child : children)
	{
		if (child.getTagName() == "DW_TAG_member")
		{
			// Add a comma before adding the next member variable
			if (counter++ > 0) values += ", ";

			Attribute type = child.getAttributeByCode(DW_AT_type);
			Attribute member_location = child.getAttributeByCode(DW_AT_data_member_location);
			uint64_t member_address = address + member_location.getUnsigned();
			Attribute name = child.getAttributeByCode(DW_AT_name);

			// Append member variable name and value to the return string
			values += name.getString();
			values += "=";
			values += deduce(member_address, *(debug_data->info()->getDIEByOffset(type.getOffset())));
		}
	}

	values += "}";

	return values;
}

std::string ValueDeducer::deduceClass(uint64_t address, DIE &class_die)
{
	assert(class_die.getTagName() == "DW_TAG_class_type");

	std::string values = "{";

	uint64_t counter = 0;
	std::vector<DIE> children = class_die.getChildren();
	for (auto &child : children)
	{
		if (child.getTagName() == "DW_TAG_member")
		{
			// Add a comma before adding the next member variable
			if (counter++ > 0) values += ", ";

			Attribute type = child.getAttributeByCode(DW_AT_type);
			Attribute member_location = child.getAttributeByCode(DW_AT_data_member_location);
			uint64_t member_address = address + member_location.getUnsigned();
			Attribute name = child.getAttributeByCode(DW_AT_name);

			// Append member variable name and value to the return string
			values += name.getString();
			values += "=";
			values += deduce(member_address, *(debug_data->info()->getDIEByOffset(type.getOffset())));
		}
	}

	values += "}";

	return values;
}

std::string ValueDeducer::deduceConst(uint64_t address, const DIE &const_die)
{
	// Get the type
	Attribute type = const_die.getAttributeByCode(DW_AT_type);
	DIE type_die = *(debug_data->info()->getDIEByOffset(type.getOffset()));
	return deduce(address, type_die);
}