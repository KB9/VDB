#pragma once

#include <dwarf.h>
#include "libdwarf.h"

#include <memory>

#include "DebuggingInformationEntry.hpp"
#include "DIECompileUnit.hpp"

#include "DIESubprogram.hpp"
#include <typeinfo>
#include <iostream>

template <class T>
using SharedPtrVector = std::vector<std::shared_ptr<T>>;

// Compilation unit header
// This represents one main source code file from the codebase
class CUHeader
{
public:
	CUHeader(const Dwarf_Debug &dbg);

	template <typename T>
	SharedPtrVector<T> getDIEsOfType()
	{
		// Get all children in the DIE hierarchy in a vector
		SharedPtrVector<DebuggingInformationEntry> all_children;
		getChildrenOf(root_die, all_children);

		SharedPtrVector<T> results;
		for (std::shared_ptr<DebuggingInformationEntry> &child : all_children)
		{
			// If the type of the child matches the specified type
			if (typeid(*child) == typeid(T))
			{
				std::shared_ptr<T> casted_child = std::static_pointer_cast<T>(child);
				results.push_back(casted_child);
			}
		}

		return results;
	}

	std::shared_ptr<DIECompileUnit> root_die;
	
	Dwarf_Unsigned length;
	Dwarf_Half version_stamp;
	Dwarf_Unsigned abbrev_offset;
	Dwarf_Half address_size;
	Dwarf_Unsigned next_cu_header;

private:
	// Recursive method to get all children (below the specified DIE) in the DIE
	// hierarchy and place them in a vector.
	void getChildrenOf(
		std::shared_ptr<DebuggingInformationEntry> root_die,
		SharedPtrVector<DebuggingInformationEntry> &children);
};