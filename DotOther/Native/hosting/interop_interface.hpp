/**
 * \file hosting/interop_interface.hpp 
 **/
#ifndef DOTOTHER_NATIVE_INTEROP_INTERFACE_HPP
#define DOTOTHER_NATIVE_INTEROP_INTERFACE_HPP

#include <cstdint>
#include <map>

#include "core/dotother_defines.hpp"
#include "hosting/native_object.hpp"
#include "hosting/native_string.hpp"
#include "hosting/garbage_collector.hpp"

namespace dotother {

  struct InternalCall {
    const dochar* name;
    void* native_function;
  };

  using CreateAssemblyLoadContext = int32_t(*)(NString);
  using UnloadAssemblyLoadContext = void(*)(int32_t);
  using LoadAssembly = int32_t(*)(int32_t, NString);
  using GetLastLoadStatus = AssemblyLoadStatus(*)();
  using GetAssemblyName = NString(*)(int32_t);

#pragma region InteropInterface

  using GetAsmTypes = void(*)(int32_t, int32_t*, int32_t*);
  using GetTypeId = void (*)(NString, int32_t*);
  using GetFullTypeName = NString(*)(int32_t);
  using GetAsmQualifiedName = NString(*)(int32_t);
  using GetBaseType = void(*)(int32_t, int32_t*);
  using GetTypeSize = int32_t(*)(int32_t);
  using IsTypeDerivedFrom = nbool32(*)(int32_t, int32_t);
  using IsAssignableTo = nbool32(*)(int32_t, int32_t);
  using IsAssignableFrom = nbool32(*)(int32_t, int32_t);
  using IsTypeSZArray = nbool32 (*)(int32_t);
  using GetElementType = void(*)(int32_t, int32_t*);
  using GetTypeMethods = void(*)(int32_t, int32_t*, int32_t*);
  using GetTypeFields = void(*)(int32_t, int32_t*, int32_t*);
  using GetTypeProperties = void(*)(int32_t, int32_t*, int32_t*);
  using HasTypeAttribute = nbool32(*)(int32_t, int32_t);
  using GetTypeAttributes = void (*)(int32_t, int32_t*, int32_t*);
  using GetTypeManagedType = ManagedType(*)(int32_t);

#pragma endregion

#pragma region Field
  using GetFieldName = NString(*)(int32_t);
  using GetFieldType = void(*)(int32_t, int32_t*);
  using GetFieldAccessibility = TypeAccessibility(*)(int32_t);
  using GetFieldAttributes = void(*)(int32_t, int32_t*, int32_t*);
#pragma endregion

#pragma region Property
  using GetPropertyName = NString(*)(int32_t);
  using GetPropertyType = void(*)(int32_t, int32_t*);
  using GetPropertyAttributes = void(*)(int32_t, int32_t*, int32_t*);
#pragma endregion

#pragma region Attribute
  using GetAttributeValue = void(*)(int32_t, NString, void*);
  using GetAttributeType = void(*)(int32_t, int32_t*);
#pragma endregion

#pragma region Method
  using GetMethodName = NString(*)(int32_t);
  using GetMethodReturnType = void(*)(int32_t, int32_t*);
  using GetMethodParameterTypes = void(*)(int32_t, int32_t*, int32_t*);
  using GetMethodAccessibility = TypeAccessibility(*)(int32_t);
  using GetMethodAttributes = void(*)(int32_t, int32_t*, int32_t*);
#pragma endregion

  using SetInternalCalls = void(*)(void*, int32_t);
  using SetInternalCall = void(*)(InternalCall);
  
  using CreateObject = void*(*)(int32_t, nbool32, const void**, const ManagedType*, int32_t);
  using DestroyObject = void(*)(void*);
  
  using InvokeMethod = void(*)(void*, NString, const void**, const ManagedType*, int32_t);
  using InvokeMethodRet = void(*)(void*, NString, const void**, const ManagedType*, int32_t, void*);
  
  using InvokeStaticMethod = void (*)(int32_t, NString, const void**, const ManagedType*, int32_t);
  using InvokeStaticMethodRet = void (*)(int32_t, NString, const void**, const ManagedType*, int32_t, void*);
  
  using SetField = void(*)(void*, NString, void*);
  using GetField = void(*)(void*, NString, void*);
  
  using SetProperty = void(*)(void*, NString, void*);
  using GetProperty = void(*)(void*, NString, void*);
  
  using CollectGarbage = void(*)(int32_t, GCMode, nbool32, nbool32);
  using WaitForPendingFinalizers = void(*)();

namespace interface_bindings {

  struct FunctionTable {
    CreateAssemblyLoadContext create_assembly_load_context = nullptr;
    UnloadAssemblyLoadContext unload_assembly_load_context = nullptr;
    LoadAssembly load_assembly = nullptr;
    GetLastLoadStatus get_last_load_status = nullptr;
    GetAssemblyName get_assembly_name = nullptr;

#pragma region TypeInterface

    GetAsmTypes get_asm_types = nullptr;
    GetTypeId get_type_id = nullptr;
    GetFullTypeName get_full_type_name = nullptr;
    GetAsmQualifiedName get_asm_qualified_name = nullptr;
    GetBaseType get_base_type = nullptr;
    GetTypeSize get_type_size = nullptr;
    IsTypeDerivedFrom is_type_derived_from = nullptr;
    IsAssignableTo is_assignable_to = nullptr;
    IsAssignableFrom is_assignable_from = nullptr;
    IsTypeSZArray is_type_sz_array = nullptr;
    GetElementType get_element_type = nullptr;
    GetTypeMethods get_type_methods = nullptr;
    GetTypeFields get_type_fields = nullptr;
    GetTypeProperties get_type_properties = nullptr;
    HasTypeAttribute has_type_attribute = nullptr;
    GetTypeAttributes get_type_attributes = nullptr;
    GetTypeManagedType get_type_managed_type = nullptr;

#pragma endregion
		
#pragma region Field
    GetFieldName get_field_name = nullptr;
    GetFieldType get_field_type = nullptr;
    GetFieldAccessibility get_field_accessibility = nullptr;
    GetFieldAttributes get_field_attributes = nullptr;
#pragma endregion

#pragma region Property
    GetPropertyName get_property_name = nullptr;
    GetPropertyType get_property_type = nullptr;
    GetPropertyAttributes get_property_attributes = nullptr;
#pragma endregion

#pragma region Attribute
    GetAttributeValue get_attr_value = nullptr;
    GetAttributeType get_attr_type = nullptr;
#pragma endregion

#pragma region Method
    GetMethodName get_method_name = nullptr;
    GetMethodReturnType get_method_return_type = nullptr;
    GetMethodParameterTypes get_method_param_types = nullptr;
    GetMethodAttributes get_method_attributes = nullptr;
    GetMethodAccessibility get_method_accessibility = nullptr;
#pragma endregion

    SetInternalCalls set_internal_calls = nullptr;
    SetInternalCall set_internal_call = nullptr;
    
    CreateObject create_object = nullptr;
    DestroyObject destroy_object = nullptr;
    
    InvokeMethod invoke_method = nullptr;
    InvokeMethodRet invoke_method_ret = nullptr;
    
    InvokeStaticMethod invoke_static_method = nullptr;
    InvokeStaticMethodRet invoke_static_method_ret = nullptr;
    
    SetField set_field = nullptr;
    GetField get_field = nullptr;
    
    SetProperty set_property = nullptr;
    GetProperty get_property = nullptr;
    
    CollectGarbage collect_garbage = nullptr;
    WaitForPendingFinalizers wait_for_pending_finalizers = nullptr;
    
    bool BoundToAsm() const;
    
    constexpr FunctionTable() {
    }
  };

} // namespace interface_bindings

  struct MethodKey {
    NString method_name;
    uint64_t type_id = 0;
    uint64_t object_handle = 0;
    

    MethodKey() {}
    MethodKey(NString method_name, uint64_t type_id, uint64_t object_handle) 
      : method_name(method_name), type_id(type_id), object_handle(object_handle) {}
  };

} // namespace dotother

template <>
struct std::less<dotother::MethodKey> {
  std::size_t operator()(const dotother::MethodKey& lhs, const dotother::MethodKey& rhs) const {
    if (lhs.type_id < rhs.type_id) {
      return true;
    }

    if (lhs.object_handle < rhs.object_handle) {
      return true;
    }

    return (std::string)lhs.method_name < (std::string)rhs.method_name;
  }
};

namespace dotother {

  class NObject;

  class InteropInterface {
    public:
      static InteropInterface& Instance();
      static void Unbind();
      
      interface_bindings::FunctionTable& FunctionTable();

      void RegisterObject(uint64_t handle, NObject* object);

      void InvokeNativeFunction(uint64_t handle, const std::string_view method_name);
    
    private:
      InteropInterface() = default;
      ~InteropInterface() = default;

      InteropInterface(InteropInterface&&) = delete;
      InteropInterface(const InteropInterface&) = delete;
      InteropInterface& operator=(InteropInterface&&) = delete;
      InteropInterface& operator=(const InteropInterface&) = delete;
      
      static InteropInterface* instance;
      
      interface_bindings::FunctionTable interop;

      std::map<uint64_t , NObject*> registered_objects;
  };
  
  static inline interface_bindings::FunctionTable& Interop() {
    return InteropInterface::Instance().FunctionTable();
  }

} // namespace dotother

#endif // !DOTOTHER_NATIVE_INTEROP_INTERFACE_HPP
