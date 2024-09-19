/**
 * \file interop_interface.hpp 
 **/
#ifndef DOTOTHER_NATIVE_INTEROP_INTERFACE_HPP
#define DOTOTHER_NATIVE_INTEROP_INTERFACE_HPP

#include <cstdint>

#include "defines.hpp"
#include "native_string.hpp"
#include "garbage_collector.hpp"

namespace dotother {

	using SetInternalCalls = void(*)(void*, int32_t);
	using CreateAssemblyLoadContext = int32_t(*)(NString);
	using UnloadAssemblyLoadContext = void(*)(int32_t);
	using LoadManagedAssembly = int32_t(*)(int32_t, NString);
	using GetLastLoadStatus = AssemblyLoadStatus(*)();
	using GetAssemblyName = NString(*)(int32_t);

#pragma region InteropInterface

  using GetAsmTypes = void(*)(int32_t, int32_t*, int32_t*);
	using GetTypeId = void (*)(NString, int32_t*);
	using GetFullTypeName = NString(*)(int32_t);
	using GetAsmQualifiedName = NString(*)(int32_t);
	using GetBaseType = void(*)(int32_t, int32_t*);
	using GetTypeSize = int32_t(*)(int32_t);
	using IsTypeSubclassOf = nbool32(*)(int32_t, int32_t);
	using IsTypeAssignableTo = nbool32(*)(int32_t, int32_t);
	using IsTypeAssignableFrom = nbool32(*)(int32_t, int32_t);
	using IsTypeSZArray = nbool32 (*)(int32_t);
	using GetElementType = void(*)(int32_t, int32_t*);
	using GetTypeMethods = void(*)(int32_t, int32_t*, int32_t*);
	using GetTypeFields = void(*)(int32_t, int32_t*, int32_t*);
	using GetTypeProperties = void(*)(int32_t, int32_t*, int32_t*);
	using HasTypeAttribute = nbool32(*)(int32_t, int32_t);
	using GetTypeAttributes = void (*)(int32_t, int32_t*, int32_t*);
	using GetTypeManagedType = ManagedType(*)(int32_t);

#pragma endregion

#pragma region MethodInfo
	using GetMethodInfoName = NString(*)(int32_t);
	using GetMethodInfoReturnType = void(*)(int32_t, int32_t*);
	using GetMethodInfoParameterTypes = void(*)(int32_t, int32_t*, int32_t*);
	using GetMethodInfoAccessibility = TypeAccessibility(*)(int32_t);
	using GetMethodInfoAttributes = void(*)(int32_t, int32_t*, int32_t*);
#pragma endregion

#pragma region FieldInfo
	using GetFieldInfoName = NString(*)(int32_t);
	using GetFieldInfoType = void(*)(int32_t, int32_t*);
	using GetFieldInfoAccessibility = TypeAccessibility(*)(int32_t);
	using GetFieldInfoAttributes = void(*)(int32_t, int32_t*, int32_t*);
#pragma endregion

#pragma region PropertyInfo
	using GetPropertyInfoName = NString(*)(int32_t);
	using GetPropertyInfoType = void(*)(int32_t, int32_t*);
	using GetPropertyInfoAttributes = void(*)(int32_t, int32_t*, int32_t*);
#pragma endregion

#pragma region Attribute
	using GetAttributeFieldValue = void(*)(int32_t, NString, void*);
	using GetAttributeType = void(*)(int32_t, int32_t*);
#pragma endregion

	using CreateObject = void*(*)(int32_t, nbool32, const void**, const ManagedType*, int32_t);
	using InvokeMethod = void(*)(void*, NString, const void**, const ManagedType*, int32_t);
	using InvokeMethodRet = void(*)(void*, NString, const void**, const ManagedType*, int32_t, void*);
	using InvokeStaticMethod = void (*)(int32_t, NString, const void**, const ManagedType*, int32_t);
	using InvokeStaticMethodRet = void (*)(int32_t, NString, const void**, const ManagedType*, int32_t, void*);
	using SetFieldValue = void(*)(void*, NString, void*);
	using GetFieldValue = void(*)(void*, NString, void*);
	using SetPropertyValue = void(*)(void*, NString, void*);
	using GetPropertyValue = void(*)(void*, NString, void*);
	using DestroyObject = void(*)(void*);

	using CollectGarbage = void(*)(int32_t, GCMode, nbool32, nbool32);
	using WaitForPendingFinalizers = void(*)();

  struct FunctionTable {

		// CollectGarbage CollectGarbage = nullptr;
		// WaitForPendingFinalizers WaitForPendingFinalizers = nullptr;
  };

namespace interop {

    static SetInternalCalls set_internal_calls = nullptr;
    static CreateAssemblyLoadContext create_assembly_load_context = nullptr;
    static UnloadAssemblyLoadContext unload_assembly_load_context = nullptr;
    static LoadManagedAssembly load_managed_assembly = nullptr;
    static GetLastLoadStatus get_last_load_status = nullptr;
    static GetAssemblyName get_assembly_name = nullptr;

#pragma region TypeInterface

		static GetAsmTypes get_asm_types = nullptr;
		static GetTypeId get_type_id = nullptr;
		static GetFullTypeName get_full_type_name = nullptr;
		static GetAsmQualifiedName get_asm_qualified_name = nullptr;
		static GetBaseType get_base_type = nullptr;
		static GetTypeSize get_type_size = nullptr;
		static IsTypeSubclassOf is_type_subclass_of = nullptr;
		static IsTypeAssignableTo is_type_assignable_to = nullptr;
		static IsTypeAssignableFrom is_type_assignable_from = nullptr;
		static IsTypeSZArray is_type_sz_array = nullptr;
		static GetElementType get_element_type = nullptr;
		static GetTypeMethods get_type_methods = nullptr;
		static GetTypeFields get_type_fields = nullptr;
		static GetTypeProperties get_type_properties = nullptr;
		static HasTypeAttribute has_type_attribute = nullptr;
		static GetTypeAttributes get_type_attributes = nullptr;
		static GetTypeManagedType get_type_managed_type = nullptr;

#pragma endregion

#pragma region MethodInfo
		static GetMethodInfoName GetMethodInfoName = nullptr;
		static GetMethodInfoReturnType GetMethodInfoReturnType = nullptr;
		static GetMethodInfoParameterTypes GetMethodInfoParameterTypes = nullptr;
		static GetMethodInfoAccessibility GetMethodInfoAccessibility = nullptr;
		static GetMethodInfoAttributes GetMethodInfoAttributes = nullptr;
#pragma endregion
		
#pragma region FieldInfo
		static GetFieldInfoName GetFieldInfoName = nullptr;
		static GetFieldInfoType GetFieldInfoType = nullptr;
		static GetFieldInfoAccessibility GetFieldInfoAccessibility = nullptr;
		static GetFieldInfoAttributes GetFieldInfoAttributes = nullptr;
#pragma endregion

#pragma region PropertyInfo
		static GetPropertyInfoName GetPropertyInfoName = nullptr;
		static GetPropertyInfoType GetPropertyInfoType = nullptr;
		static GetPropertyInfoAttributes GetPropertyInfoAttributes = nullptr;
#pragma endregion

#pragma region Attribute
		static GetAttributeFieldValue GetAttributeFieldValue = nullptr;
		static GetAttributeType GetAttributeType = nullptr;
#pragma endregion

		static CreateObject create_object = nullptr;
		static CreateAssemblyLoadContext create_asm_load_ctx = nullptr;
		static InvokeMethod invoke_method = nullptr;
		static InvokeMethodRet invoke_method_ret = nullptr;
		static InvokeStaticMethod invoke_static_method = nullptr;
		static InvokeStaticMethodRet invoke_static_method_ret = nullptr;
		static SetFieldValue set_field = nullptr;
		static GetFieldValue fet_field = nullptr;
		static SetPropertyValue set_property = nullptr;
		static GetPropertyValue get_property = nullptr;
		static DestroyObject destroy_object = nullptr;

    static CollectGarbage collect_garbage = nullptr;
    static WaitForPendingFinalizers wait_for_pending_finalizers = nullptr;


} // namespace interop
} // namespace dotother

#endif // !DOTOTHER_NATIVE_INTEROP_INTERFACE_HPP