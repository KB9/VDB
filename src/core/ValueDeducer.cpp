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
	// // std::shared_ptr<DebuggingInformationEntry> die = debug_data->info()->getDIEByOffset(pointer_die.type_offset);
	// std::unique_ptr<DIE> die = debug_data->info()->getDIEByOffset(pointer_die.getOffset());
	// if (die == nullptr) return "Error retrieving type pointed to";

	// uint64_t new_address = ptrace(PTRACE_PEEKDATA, target_pid, address, 0);
	// return deduce(new_address, *die);
	return "redacted";
}

std::string ValueDeducer::deduceReference(uint64_t address, const DIE &ref_die)
{
	// // std::shared_ptr<DebuggingInformationEntry> die = debug_data->info()->getDIEByOffset(ref_die.type_offset);
	// std::unique_ptr<DIE> die = debug_data->info()->getDIEByOffset(red_die.getOffset());
	// if (die == nullptr) return "Error retrieving reference type";

	// uint64_t new_address = ptrace(PTRACE_PEEKDATA, target_pid, address, 0);
	// return deduce(new_address, *die);
	return "redacted";
}

std::string ValueDeducer::deduceArray(uint64_t address, DIE &array_die)
{
	// Get the type of the array
	// // std::shared_ptr<DebuggingInformationEntry> die = debug_data->info()->getDIEByOffset(array_die.getTypeOffset());
	// std::unique_ptr<DIE> die = debug_data->info()->getDIEByOffset(array_die.getOffset());
	// if (die == nullptr) return "Error getting array type";
	// DIEBaseType *array_base_type_die = dynamic_cast<DIEBaseType *>(die.get());

	// // Find the subrange child DIE to determine the upper bound of the array
	// uint64_t upper_bound = 0;
	// auto child_ptrs = array_die.getChildren();
	// for (auto child_ptr : child_ptrs)
	// {
	// 	if (child_ptr->getTagName() == "DW_TAG_subrange_type")
	// 	{
	// 		auto child = dynamic_cast<DIESubrangeType *>(child_ptr.get());
	// 		upper_bound = child->getUpperBound();
	// 		break;
	// 	}
	// }

	// // Deduce the values of the array's elements
	// std::string array_string = "{";
	// uint64_t array_type_size = array_base_type_die->getByteSize();
	// uint64_t array_size = array_type_size * (upper_bound + 1);
	// for (uint64_t i = 0; i < array_size; i += array_type_size)
	// {
	// 	// Add a comma before adding the next value
	// 	if (i > 0) array_string += ", ";

	// 	// Deduce the array values and add them to the string
	// 	array_string += deduce(address + i, *array_base_type_die);
	// }
	// array_string += "}";
	// return array_string;
	return "redacted";
}

std::string ValueDeducer::deduceStructure(uint64_t address, DIE &struct_die)
{
	// std::string values = "{";

	// // Get the values of all the member variables
	// uint64_t counter = 0;
	// auto child_ptrs = struct_die.getChildren();
	// for (auto child_ptr : child_ptrs)
	// {
	// 	if (child_ptr->getTagName() == "DW_TAG_member")
	// 	{
	// 		// Add a comma before adding the next member variable
	// 		if (counter++ > 0) values += ", ";

	// 		// Get the member variable DIE, its base type and its address
	// 		auto member = dynamic_cast<DIEMemberType *>(child_ptr.get());
	// 		auto member_type_die = debug_data->info()->getDIEByOffset(member->getTypeOffset());
	// 		uint64_t member_address = address + member->getDataMemberLocation();

	// 		// Append member variable name and value to the return string
	// 		values += member->getName();
	// 		values += "=";
	// 		values += deduce(member_address, *member_type_die);
	// 	}
	// }

	// values += "}";

	// return values;
	return "redacted";
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
	// // Get the type of the array
	// std::shared_ptr<DebuggingInformationEntry> die = debug_data->info()->getDIEByOffset(const_die.getTypeOffset());
	// if (die == nullptr) return "Error retrieving const variable type";
	// return deduce(address, *die);
	return "redacted";
}