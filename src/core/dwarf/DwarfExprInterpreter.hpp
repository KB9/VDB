#include <dwarf.h>
#include "libdwarf.h"

#include <stdint.h>
#include <sys/types.h>

class DwarfExprInterpreter
{
public:
	DwarfExprInterpreter(pid_t target_pid);

	// Gets the address of the variable from the two specified DWARF expressions
	uint64_t parse(uint8_t *frame_base_expr, uint8_t op_code, uint8_t *op_param);

	inline uint8_t encodeSLEB128(int64_t value);
	inline int64_t decodeSLEB128(const uint8_t *p);

private:
	pid_t target_pid;
};