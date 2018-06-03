#include "DwarfExprInterpreter.hpp"

#include <cmath>

#include "../Unwinder.hpp"

DwarfExprInterpreter::DwarfExprInterpreter(pid_t target_pid)
{
	this->target_pid = target_pid;
}

// Gets the address of the variable from the specified DWARF expressions
uint64_t DwarfExprInterpreter::parse(uint8_t *frame_base_expr, uint8_t op_code,
                                     uint8_t *op_param)
{
	if (op_code == DW_OP_addr) // 0x3
	{
		// The value is located in the .data section of the executable, so use
		// a direct address to find its value
		return decodeDataAddress(op_param);
	}
	else if (op_code == DW_OP_fbreg) // 0x91
	{
		// The value is located at an offset from the base of the stack frame
		return decodeStackFrameAddress(op_param, frame_base_expr[0]);
	}

	return 0;
}

uint8_t DwarfExprInterpreter::encodeSLEB128(int64_t value)
{
	bool more;
	uint8_t byte;
	do
	{
		byte = value & 0x7F;
		value >>= 7;
		more = !((((value == 0 ) && ((byte & 0x40) == 0)) ||
		          ((value == -1) && ((byte & 0x40) != 0))));
		if (more)
			byte |= 0x80;
	} while (more);

	return byte;
}

int64_t DwarfExprInterpreter::decodeSLEB128(const uint8_t *p)
{
	int64_t value = 0;
	unsigned shift = 0;
	uint8_t byte;
	do
	{
		byte = *p++;
		value |= ((byte & 0x7F) << shift);
		shift += 7;
	} while (byte >= 128);

	// Sign extend negative numbers
	if (byte & 0x40)
		value |= (-1ULL) << shift;

	return value;
}

uint64_t DwarfExprInterpreter::decodeDataAddress(uint8_t *op_param)
{
	// The address value is read in reverse, so multiply each byte of
	// the address by 256^n (where n represents the amount of bytes that
	// were read before the current byte) before adding to the total
	// address value
	uint64_t addr = 0;
	size_t idx = 0;
	while (op_param[idx] != 0)
	{
		uint64_t multiplier = std::pow(256, idx);
		addr += (multiplier * op_param[idx]);
		++idx;
	}
	return addr;
}

uint64_t DwarfExprInterpreter::decodeStackFrameAddress(uint8_t *op_param,
                                                       uint8_t frame_base_op_code)
{
	// Use the Canonical Frame Address (CFA) to get this value
	if (frame_base_op_code == DW_OP_call_frame_cfa) // 0x9C
	{
		// Decode the SLEB128-encoded parameter
		int64_t offset = decodeSLEB128(op_param);

		// Get the CFA and apply the offset
		Unwinder unwinder(target_pid);
		unwinder.unwindStep();
		uint64_t cfa = unwinder.getRegisterValue(UNW_X86_64_CFA);
		uint64_t var_addr = cfa + offset;

		return var_addr;
	}
}