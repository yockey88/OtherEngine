/**
  * \file host.cpp
  */
#include "host.hpp"

#include <cstdint>
#include <optional>
#include <ostream>

#include <stdexcept>
#include <string>
#include <filesystem> 
#include <iostream>

#include <spdlog/fmt/fmt.h>
#include <wincrypt.h>

#include "assembly.hpp"
#include "coreclr_delegates.h"

#include "defines.hpp"
#include "utilities.hpp"
#include "interop_interface.hpp"
#include "native_string.hpp"

using namespace dotother::literals;

namespace dotother {

  struct CoreClrFunctions {
    hostfxr_set_error_writer_fn set_error_writer = nullptr;
    hostfxr_initialize_for_dotnet_command_line_fn init_host_cmd_line = nullptr;
    hostfxr_initialize_for_runtime_config_fn init_host_config = nullptr;
    hostfxr_get_runtime_delegate_fn get_runtime_delegate = nullptr;
    hostfxr_close_fn close_host_fxr = nullptr; 
    load_assembly_and_get_function_pointer_fn get_managed_function_ptr = nullptr;
  };
  static CoreClrFunctions coreclr;

  template <typename Fn>
  Fn LoadFunction(void* handle, const char* name) {
  #ifdef _WIN32
    auto fn = (Fn)GetProcAddress((HMODULE)handle, name);
  #else // __linux__
    auto fn = (Fn)dlsym(handle, name);
  #endif // !_WIN32
    if (fn == nullptr) {
      std::cout << "Failed to load function : " << name << std::endl;
      return nullptr;
    }
    return fn;
  }

  bool Host::LoadHost() {
    if (is_loaded) {
      return true;
    }

    if (!std::filesystem::exists(config.host_config_path)) {
      util::print(DO_STR("Host config path does not exist"));
      return false;
    }

    if (!std::filesystem::exists(config.managed_asm_path)) {
      util::print(DO_STR("Managed assembly path does not exist"));
      return false;
    }

    if (config.log_callback == nullptr) {
      util::print(DO_STR("Logging callback not registered for host"));
      return false;
    }

    if (config.exception_callback == nullptr) {
      util::print(DO_STR("Excpeption callback not registered for host"));
      return false;
    }

    is_loaded = LoadClrFunctions();
    if (!is_loaded) {
      return false;
    }

    if (config.coreclr_error_callback != nullptr) {
      coreclr.set_error_writer(config.coreclr_error_callback);
    } else {
      coreclr.set_error_writer([](const dochar* message) {
#ifdef DOTOTHER_WIDE_CHARS
        dostring_view message_view(message);
        std::string result = util::WideToChar(message_view); 
#else
        std::string result = std::string{ message };
#endif
        util::print(DO_STR("CoreCLR error: {}"), result);
      });
    }

    if (config.internal_logging_hook != nullptr) {
      util::GetUtils().log_sink = config.internal_logging_hook;
    } else {
      util::GetUtils().log_sink = [](const std::string_view message, dotother::MessageLevel level, bool verbose) {
        if (!verbose) {
          return;
        }

		    std::cout << message << std::endl;
      };
    }

    is_loaded = InitializeHost();
    if (!is_loaded) {
      util::print(DO_STR("Failed to initialize host, unloading..."));
      UnloadHost();
      return false;
    }

    util::GetUtils().SetLogLevel(config.log_level);

    util::print(DO_STR("Host loaded successfully"));
    is_loaded = true;
    return true;
  }

  void Host::CallEntryPoint(/* const DotOtherArgs */) {
    if (!is_loaded) {
      util::print(DO_STR("Host is not loaded, can not call entry point"));
      throw std::runtime_error("Host is not loaded, can not call entry point");
    }
    
    if (host_calls.entry == nullptr) {
      util::print(DO_STR("Entry point is null"));
      throw std::runtime_error("Entry point is null");
    }  

    LoadManagedFunctions();

    DotOtherArgs args = {
      .exception_callback = config.exception_callback,
      .log_callback = config.log_callback,
    };
    host_calls.entry(args);
  }

  void Host::UnloadHost() {
    if (!is_loaded) {
      return;
    }
    
    coreclr.close_host_fxr(host_fxr);

    host_calls.entry = nullptr;
    host_fxr = nullptr;
    is_loaded = false;
    
    config.exception_callback = nullptr;
    config.log_callback = nullptr;
    config.host_config_path.clear();
    config.managed_asm_path.clear();
    config.dotnet_type.clear();
    config.entry_point.clear();
    config.delegate_type = nullptr;

    managed_asm.clear();

    coreclr.init_host_cmd_line = nullptr;
    coreclr.init_host_config = nullptr;
    coreclr.get_runtime_delegate = nullptr;
    coreclr.close_host_fxr = nullptr;
    coreclr.set_error_writer = nullptr;
    coreclr.get_managed_function_ptr = nullptr;
  }

  AssemblyContext Host::CreateAsmContext(const std::string_view name) {
    if (!Interop().BoundToAsm()) {
      util::print(DO_STR("Interop interface not bound"));
      throw std::runtime_error("Interop interface not bound");
    }

    NScopedString ctx_name = NString::New(name);
    AssemblyContext ctx(this);
    
    ctx.context_id = Interop().create_assembly_load_context(ctx_name);
    ctx.host = this;
    
    return ctx;
  }

  void Host::UnloadAssemblyContext(AssemblyContext& InLoadContext) {
    if (!Interop().BoundToAsm()) {
      util::print(DO_STR("Interop interface not bound"));
      throw std::runtime_error("Interop interface not bound");
    }

    Interop().unload_assembly_load_context(InLoadContext.context_id);
    InLoadContext.context_id = -1;
    InLoadContext.assemblies.clear();
  }
  
  interface_bindings::FunctionTable& Host::GetInteropInterface() {
    return InteropInterface::Instance().FunctionTable();
  }

  bool Host::LoadClrFunctions() {
    std::optional<std::filesystem::path> host_path = GetHostPath();
    if (!host_path.has_value()) {
      util::print(DO_STR("Failed to get host path"));
      return false;
    }

    void* hostfxr_lib = LoadLibraryW(host_path->c_str());
    if (hostfxr_lib == nullptr) {
      util::print(DO_STR("Failed to load hostfxr"));
      return false;
    }

    coreclr.init_host_cmd_line = LoadFunction<hostfxr_initialize_for_dotnet_command_line_fn>(hostfxr_lib, "hostfxr_initialize_for_dotnet_command_line");
    coreclr.init_host_config = LoadFunction<hostfxr_initialize_for_runtime_config_fn>(hostfxr_lib, "hostfxr_initialize_for_runtime_config");
    coreclr.get_runtime_delegate = LoadFunction<hostfxr_get_runtime_delegate_fn>(hostfxr_lib, "hostfxr_get_runtime_delegate");
    // coreclr.run_app = LoadFunction<hostfxr_run_app_fn>(hostfxr_lib, "hostfxr_run_app");
    coreclr.close_host_fxr = LoadFunction<hostfxr_close_fn>(hostfxr_lib, "hostfxr_close");
    coreclr.set_error_writer = LoadFunction<hostfxr_set_error_writer_fn>(hostfxr_lib, "hostfxr_set_error_writer");
    
    if (coreclr.init_host_cmd_line == nullptr || coreclr.init_host_config == nullptr || 
        coreclr.get_runtime_delegate == nullptr || coreclr.close_host_fxr == nullptr) {
      util::print(DO_STR("Failed to load hostfxr functions"));
      return false;
    }
    util::print(DO_STR("Hostfxr functions loaded successfully"));

    return coreclr.init_host_cmd_line != nullptr && coreclr.init_host_config != nullptr && 
          coreclr.get_runtime_delegate != nullptr && coreclr.close_host_fxr != nullptr;
  }

  bool Host::InitializeHost() {
    hostfxr_handle host_fxr = nullptr;
    int32_t rc = coreclr.init_host_config(config.host_config_path.c_str() , nullptr , &host_fxr);
    if (rc != 0 || host_fxr == nullptr) {
      util::print(DO_STR("Could not initialize host_fxr handle : {:#08x}"), rc);
      coreclr.close_host_fxr(host_fxr);
      return false;
    }

    void* delegate = nullptr;
    rc = coreclr.get_runtime_delegate(host_fxr , hdt_load_assembly_and_get_function_pointer, &delegate);
    if (rc != 0 || delegate == nullptr) {
      util::print(DO_STR("Could not get managed function pointer : {:#08x}"), rc);
      coreclr.close_host_fxr(host_fxr);
      return false;
    } 
    coreclr.get_managed_function_ptr = (load_assembly_and_get_function_pointer_fn)delegate;
    if (coreclr.get_managed_function_ptr == nullptr) {
      util::print(DO_STR("Failed to get managed function pointer"));
      return false;
    }
    util::print(DO_STR("Managed function pointer loaded"));
    
    host_calls.entry = nullptr;
    host_calls.entry = LoadManagedFunction<EntryPoint>(config.dotnet_type.c_str(), config.entry_point.c_str());
    if (host_calls.entry == nullptr) {
      util::print(DO_STR("Failed to load entry point"));
      return 1;
    }
    util::print(DO_STR("Entry point loaded"));

    return true;
  }

  void Host::LoadManagedFunctions() {
    auto& interop = Interop();
    interop.create_assembly_load_context = LoadManagedFunction<CreateAssemblyLoadContext>(DO_STR("DotOther.Managed.AssemblyLoader, DotOther.Managed"), DO_STR("CreateAssemblyLoadContext"));
    interop.unload_assembly_load_context = LoadManagedFunction<UnloadAssemblyLoadContext>(DO_STR("DotOther.Managed.AssemblyLoader, DotOther.Managed"), DO_STR("UnloadAssemblyLoadContext"));
    interop.get_last_load_status = LoadManagedFunction<GetLastLoadStatus>(DO_STR("DotOther.Managed.AssemblyLoader, DotOther.Managed"), DO_STR("GetLastLoadStatus"));
    interop.load_assembly = LoadManagedFunction<LoadAssembly>(DO_STR("DotOther.Managed.AssemblyLoader, DotOther.Managed"), DO_STR("LoadAssembly"));
    interop.get_assembly_name = LoadManagedFunction<GetAssemblyName>(DO_STR("DotOther.Managed.AssemblyLoader, DotOther.Managed"), DO_STR("GetAsmName"));

    interop.get_asm_types = LoadManagedFunction<GetAsmTypes>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetAsmTypes"));
    interop.get_type_id = LoadManagedFunction<GetTypeId>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetTypeId"));
    interop.get_full_type_name = LoadManagedFunction<GetFullTypeName>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetFullTypeName"));
    interop.get_asm_qualified_name = LoadManagedFunction<GetAsmQualifiedName>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetAsmQualifiedName"));
    interop.get_base_type = LoadManagedFunction<GetBaseType>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetBaseType"));
    interop.get_type_size = LoadManagedFunction<GetTypeSize>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetTypeSize"));
    interop.is_type_derived_from = LoadManagedFunction<IsTypeDerivedFrom>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("IsTypeDerivedFrom"));
    interop.is_assignable_to = LoadManagedFunction<IsAssignableTo>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("IsAssignableTo"));
    interop.is_assignable_from = LoadManagedFunction<IsAssignableFrom>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("IsAssignableFrom"));
    interop.is_type_sz_array = LoadManagedFunction<IsTypeSZArray>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("IsSzArray"));
    interop.get_element_type = LoadManagedFunction<GetElementType>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetElementType"));
    interop.get_type_methods = LoadManagedFunction<GetTypeMethods>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetTypeMethods"));
    interop.get_type_fields = LoadManagedFunction<GetTypeFields>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetTypeFields"));
    interop.get_type_properties = LoadManagedFunction<GetTypeProperties>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetTypeProperties"));
    interop.has_type_attribute = LoadManagedFunction<HasTypeAttribute>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("HasAttribute"));
    interop.get_type_attributes = LoadManagedFunction<GetTypeAttributes>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetAttributes"));
    interop.get_type_managed_type = LoadManagedFunction<GetTypeManagedType>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetTypeManagedType"));

    interop.get_field_name = LoadManagedFunction<GetFieldName>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetFieldName"));
    interop.get_field_type = LoadManagedFunction<GetFieldType>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetFieldType"));
    interop.get_field_accessibility = LoadManagedFunction<GetFieldAccessibility>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetFieldAccessibility"));
    interop.get_field_attributes = LoadManagedFunction<GetFieldAttributes>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetFieldAttributes"));

    interop.get_property_name = LoadManagedFunction<GetPropertyName>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetPropertyName"));
    interop.get_property_info = LoadManagedFunction<GetPropertyType>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetPropertyType"));
    interop.get_property_attributes = LoadManagedFunction<GetPropertyAttributes>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetPropertyAttributes"));

    interop.get_attr_value = LoadManagedFunction<GetAttributeValue>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetAttributeValue"));
    interop.get_attr_type = LoadManagedFunction<GetAttributeType>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetAttributeType"));

    interop.get_method_name = LoadManagedFunction<GetMethodName>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetMethodName"));
    interop.get_method_return_type = LoadManagedFunction<GetMethodReturnType>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetMethodReturnType"));
    interop.get_method_param_types = LoadManagedFunction<GetMethodParameterTypes>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetMethodParameterTypes"));
    interop.get_method_attributes = LoadManagedFunction<GetMethodAttributes>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetMethodAttributes"));
    interop.get_method_accessibility = LoadManagedFunction<GetMethodAccessibility>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetMethodAccessibility"));

    interop.create_object = LoadManagedFunction<CreateObject>(DO_STR("DotOther.Managed.ManagedObject, DotOther.Managed"), DO_STR("CreateObject"));
    interop.destroy_object = LoadManagedFunction<DestroyObject>(DO_STR("DotOther.Managed.ManagedObject, DotOther.Managed"), DO_STR("DestroyObject"));

    interop.invoke_method = LoadManagedFunction<InvokeMethod>(DO_STR("DotOther.Managed.ManagedObject, DotOther.Managed"), DO_STR("InvokeMethod"));
    interop.invoke_method_ret = LoadManagedFunction<InvokeMethodRet>(DO_STR("DotOther.Managed.ManagedObject, DotOther.Managed"), DO_STR("InvokeMethodRet"));

    interop.invoke_static_method = LoadManagedFunction<InvokeStaticMethod>(DO_STR("DotOther.Managed.ManagedObject, DotOther.Managed"), DO_STR("InvokeStaticMethod"));
    interop.invoke_static_method_ret = LoadManagedFunction<InvokeStaticMethodRet>(DO_STR("DotOther.Managed.ManagedObject, DotOther.Managed"), DO_STR("InvokeStaticMethodRet"));

    interop.set_field = LoadManagedFunction<SetField>(DO_STR("DotOther.Managed.ManagedObject, DotOther.Managed"), DO_STR("SetField"));
    interop.get_field = LoadManagedFunction<GetField>(DO_STR("DotOther.Managed.ManagedObject, DotOther.Managed"), DO_STR("GetField"));

    interop.set_property = LoadManagedFunction<SetProperty>(DO_STR("DotOther.Managed.ManagedObject, DotOther.Managed"), DO_STR("SetProperty"));
    interop.get_property = LoadManagedFunction<GetProperty>(DO_STR("DotOther.Managed.ManagedObject, DotOther.Managed"), DO_STR("GetProperty"));

    interop.collect_garbage = LoadManagedFunction<CollectGarbage>(DO_STR("DotOther.Managed.GarbageCollector, DotOther.Managed"), DO_STR("CollectGarbage"));
    interop.wait_for_pending_finalizers = LoadManagedFunction<WaitForPendingFinalizers>(DO_STR("DotOther.Managed.GarbageCollector, DotOther.Managed"), DO_STR("WaitForPendingFinalizers"));

    if (!interop.BoundToAsm()) {
      util::print(DO_STR("Failed to load managed functions, runtime not bound to assembly"));
      throw std::runtime_error("Failed to load managed functions, runtime not bound to assembly");
    }
  }

  std::optional<std::filesystem::path> Host::GetHostPath() {
    get_hostfxr_parameters hostfxr_params = { 
      sizeof(get_hostfxr_parameters) , 
      nullptr , 
      nullptr 
    };
    wchar_t hostfxr_path[MAX_PATH];
    size_t buffer_size = sizeof(hostfxr_path) / sizeof(hostfxr_path[0]);
    int32_t rc = get_hostfxr_path(hostfxr_path , &buffer_size, &hostfxr_params);
    if (rc != 0) {
      util::print(DO_STR("Failed to get hostfxr path : {}"), rc);
      return std::nullopt;
    }

    std::filesystem::path host_path = hostfxr_path;
    if (!std::filesystem::exists(host_path)) {
      util::print(DO_STR("Hostfxr path does not exist"));
      return std::nullopt;
    }

    return host_path;
  }

  void* Host::LoadManagedFunction(const std::filesystem::path& asm_path, const dostring_t& type_name, const dostring_t& method_name, const dochar* delegate_type) const {
    if (!std::filesystem::exists(asm_path)) {
      using namespace std::string_view_literals;
      util::print(DO_STR("Assembly {} does not exist!"sv), asm_path.string());
      return nullptr;
    }

    void* handle = nullptr;
    int32_t rc = coreclr.get_managed_function_ptr(asm_path.c_str(), type_name.c_str(), method_name.c_str(), delegate_type, nullptr, &handle);
    if (rc == -1 || handle == nullptr) {
      util::print(DO_STR("Failed to load managed function {} ({}) | error code : {}"), 
#ifdef DOTOTHER_WIDE_CHARS
        util::WideToChar(method_name), util::WideToChar(type_name),
#else
        method_name, type_name,
#endif    
        rc
      );
      return nullptr;
    } else {
      util::print(DO_STR("Managed function {} ({}) loaded successfully"),
#ifdef DOTOTHER_WIDE_CHARS
        util::WideToChar(method_name), util::WideToChar(type_name)
#else
        method_name, type_name
#endif
      );
    }

    return handle;
  }

} // namespace dotother
