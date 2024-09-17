/**
 * \file interop_interface.hpp 
 **/
#ifndef DOTOTHER_NATIVE_INTEROP_INTERFACE_HPP
#define DOTOTHER_NATIVE_INTEROP_INTERFACE_HPP

#include <cstdint>

#include "native_string.hpp"

namespace dotother {

	using SetInternalCallsFn = void(*)(void*, int32_t);
	using CreateAssemblyLoadContextFn = int32_t(*)(NString);
	using UnloadAssemblyLoadContextFn = void(*)(int32_t);
	using LoadManagedAssemblyFn = int32_t(*)(int32_t, NString);
	// using GetLastLoadStatusFn = AssemblyLoadStatus(*)();
	using GetAssemblyNameFn = NString(*)(int32_t);

  struct FunctionTable {
    SetInternalCallsFn set_internal_calls = nullptr;
    CreateAssemblyLoadContextFn create_assembly_load_context = nullptr;
    UnloadAssemblyLoadContextFn unload_assembly_load_context = nullptr;
    LoadManagedAssemblyFn load_managed_assembly = nullptr;
    // GetLastLoadStatusFn get_last_load_status = nullptr;
    GetAssemblyNameFn get_assembly_name = nullptr;
  };

namespace interop {

  static inline FunctionTable function_table;

} // namespace interop
} // namespace dotother

#endif // !DOTOTHER_NATIVE_INTEROP_INTERFACE_HPP