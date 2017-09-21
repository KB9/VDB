#pragma once

#include <dwarf.h>
#include "libdwarf.h"

#include <memory>
#include <vector>
#include <algorithm>
#include <type_traits>

#include "DebuggingInformationEntry.hpp"
#include "DIECompileUnit.hpp"

#include "DIESubprogram.hpp"
#include <typeinfo>
#include <iostream>
#include <cstring>

// TODO: Needs a name change/re-structure for type DIE offset to make sense
struct VariableLocExpr
{
	uint64_t type_die_offset;

	uint8_t frame_base;
	uint8_t location_op;
	uint8_t *location_param;

	std::shared_ptr<DebuggingInformationEntry> die = nullptr;
};

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

	// Returns a vector, which has vector elements. These elements contain the
	// location expressions for all DIEs which match the variable name and one
	// of the DIE types specified. 
	//
	// NOTE: This function checks all variables within the DIE tree which match
	// the specified type, regardless of their lexical scope within this
	// compilation unit's source.
	template <typename... DIEs>
	std::vector<VariableLocExpr> getLocExprsFromVarName(const char *var_name)
	{
		// Get location expressions for all types in the parameter pack
		std::vector<std::vector<VariableLocExpr>> expr_vecs({ getLocExprsFromVarName_<DIEs>(var_name)... });

		// Flatten the 2D vector of location expression vectors
		std::vector<VariableLocExpr> flattened;
		for (const auto &expr_vec : expr_vecs)
		{
			flattened.insert(flattened.end(), expr_vec.begin(), expr_vec.end());
		}

		// Remove all not-found location expressions
		std::remove_if(flattened.begin(), flattened.end(),
		               [](const VariableLocExpr &expr)
		{
			return expr.frame_base == 0 &&
					expr.location_op == 0 &&
					expr.location_param == 0;
		});

		return flattened;
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

	// Primary template (false by default)
	template <typename T, typename U = int>
	struct HasLocationData : std::false_type {};

	// Specialization for U = int (true when it has specified members)
	template <typename T>
	struct HasLocationData<T, decltype((void)T::name,
	                                   (void)T::location_data_length,
	                                   (void)T::location_data,
	                                   0)> : std::true_type {};

	// Returns a vector of location expressions for all DIEs in the DIE tree that
	// match have the specified DIE type, and that have the following members:
	// - name
	// - location_data_length
	// - location_data
	//
	// NOTE: This function checks all variables within the DIE tree which match
	// the specified type, regardless of their lexical scope within this
	// compilation unit's source.
	template <typename DIE>
	std::vector<VariableLocExpr> getLocExprsFromVarName_(const char *var_name)
	{
		// Assert that the type has location data to retrieve
		static_assert(HasLocationData<DIE>::value,
		              "DIE does not have location data (DIE::name, DIE::location_data_length, DIE::location_data)");

		std::vector<VariableLocExpr> exprs;

		// Iterate over all DIEs of the specified type
		SharedPtrVector<DIE> die_ptrs = getDIEsOfType<DIE>();
		for (auto die_ptr : die_ptrs)
		{
			// Check to find out if 'name' members match
			if (strcmp(var_name, die_ptr->name.c_str()) == 0)
			{
				VariableLocExpr expr = {0};

				expr.type_die_offset = die_ptr->type_offset;

				// Get the first byte block
				expr.location_op = ((uint8_t *)die_ptr->location_data)[0];

				// Get the second byte block if it exists
				if (die_ptr->location_data_length > 1)
					expr.location_param = &((uint8_t *)die_ptr->location_data)[1];

				// Backtrace until a DIE which has frame base data is found
				DebuggingInformationEntry *temp = die_ptr.get();
				while (temp)
				{
					// Get the parent DIE
					temp = temp->getParent();

					// Check if it is a DIE which contains frame base data
					if (auto sub = dynamic_cast<DIESubprogram *>(temp))
					{
						expr.frame_base = ((uint8_t *)sub->frame_base_data)[0];
						break;
					}
				}

				expr.die = die_ptr;

				// Append the location expression after all retrievable data
				// has been found
				exprs.push_back(expr);
			}
		}
		return exprs;
	}
};