/**
  * \file hosting/host.cpp
  */
#include "hosting/host.hpp"

#include <cassert>
#include <cstdint>
#include <optional>
#include <ostream>

#include <stdexcept>
#include <string>
#include <filesystem> 
#include <iostream>
#include <array>

#include <spdlog/fmt/fmt.h>

#include "core/dotother_defines.hpp"

#include "core/utilities.hpp"
#include "hosting/native_string.hpp"
#include "hosting/interop_interface.hpp"
#include "hosting/assembly.hpp"

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

  Host* Host::instance = nullptr;
  
  Host* Host::Instance() {
    assert(instance != nullptr && "Host instance not set!");
    return instance;
  }

  Host* Host::Instance(const HostConfig& config) {
    if (instance == nullptr) {
      instance = new Host(config);
    }
    return instance;
  }

  void  Host::Destroy() {
    delete instance;
    instance = nullptr;
  }

  bool Host::LoadHost() {
    assert(config.has_value() && "Host configuration not set!");

    if (is_loaded) {
      return true;
    }

    if (!std::filesystem::exists(config->host_config_path)) {
      util::print(DO_STR("Host config path does not exist") , MessageLevel::CRITICAL);
      return false;
    }

    if (!std::filesystem::exists(config->managed_asm_path)) {
      util::print(DO_STR("Managed assembly path does not exist") , MessageLevel::CRITICAL);
      return false;
    }

    if (config->log_callback == nullptr) {
      util::print(DO_STR("Logging callback not registered for host") , MessageLevel::CRITICAL);
      return false;
    }

    if (config->exception_callback == nullptr) {
      util::print(DO_STR("Excpeption callback not registered for host") , MessageLevel::CRITICAL);
      return false;
    }

    is_loaded = LoadClrFunctions();
    if (!is_loaded) {
      return false;
    }

    if (config->coreclr_error_callback != nullptr) {
      coreclr.set_error_writer(config->coreclr_error_callback);
    } else {
      coreclr.set_error_writer([](const dochar* message) {
#ifdef DOTOTHER_WIDE_CHARS
        dostring_view message_view(message);
        std::string result = util::WideToChar(message_view); 
#else
        std::string result = std::string{ message };
#endif
        util::print(DO_STR("CoreCLR error: {}") , MessageLevel::ERR, result);
      });
    }

    if (config->internal_logging_hook != nullptr) {
      util::GetUtils().log_sink = config->internal_logging_hook;
    } else {
      util::GetUtils().log_sink = [](const std::string_view message, dotother::MessageLevel level, bool verbose) {
        //// default to simply writing to stdout when verbose
        if (!verbose) {
          return;
        }
        fmt::print(" [DotOther] > {} [{}]\n", message, level);
      };
    }

    if (config->invoke_native_method_hook == nullptr) {
      config->invoke_native_method_hook = [](uint64_t,const NString) {};
    }

    is_loaded = InitializeHost();
    if (!is_loaded) {
      util::print(DO_STR("Failed to initialize host, unloading...") , MessageLevel::CRITICAL);
      UnloadHost();
      return false;
    }

    util::GetUtils().SetLogLevel(config->log_level);
    util::print(DO_STR("Host loaded successfully") , MessageLevel::INFO);

    is_loaded = true;
    return true;
  }

  void Host::CallEntryPoint(/* const DotOtherArgs */) {
    assert(config.has_value() && "Host configuration not set!");

    if (!is_loaded) {
      util::print(DO_STR("Host is not loaded, can not call entry point") , MessageLevel::CRITICAL);
      throw std::runtime_error("Host is not loaded, can not call entry point");
    }
    
    if (host_calls.entry == nullptr) {
      util::print(DO_STR("Entry point is null") , MessageLevel::CRITICAL);
      throw std::runtime_error("Entry point is null");
    }  

    LoadManagedFunctions();

    DotOtherArgs args = {
      .exception_callback = config->exception_callback,
      .log_callback = config->log_callback,
      .invoke_native_method_hook = config->invoke_native_method_hook
    };
    host_calls.entry(args);
  }

  void Host::UnloadHost() {
    assert(config.has_value() && "Host configuration not set!");
    if (!is_loaded) {
      return;
    }
    
    coreclr.close_host_fxr(host_fxr);

    host_calls.entry = nullptr;
    host_fxr = nullptr;
    is_loaded = false;
    
    config->exception_callback = nullptr;
    config->log_callback = nullptr;
    config->host_config_path.clear();
    config->managed_asm_path.clear();
    config->dotnet_type.clear();
    config->entry_point.clear();
    config->delegate_type = nullptr;

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
      util::print(DO_STR("Interop interface not bound") , MessageLevel::CRITICAL);
      throw std::runtime_error("Interop interface not bound");
    }

    NScopedString ctx_name = NString::New(name);
    AssemblyContext ctx;
    ctx.context_id = Interop().create_assembly_load_context(ctx_name);
    
    return ctx;
  }

  void Host::UnloadAssemblyContext(AssemblyContext& InLoadContext) {
    if (!Interop().BoundToAsm()) {
      util::print(DO_STR("Interop interface not bound") ,  MessageLevel::CRITICAL);
      throw std::runtime_error("Interop interface not bound");
    }

    Interop().unload_assembly_load_context(InLoadContext.context_id);
    InLoadContext.context_id = -1;
    InLoadContext.assemblies.clear();
  }
  
  interface_bindings::FunctionTable& Host::GetInteropInterface() {
    return InteropInterface::Instance().FunctionTable();
  }

namespace {

  std::optional<std::filesystem::path> GetHostPath() {
#ifdef DOTOTHER_WINDOWS
    std::filesystem::path base_path = "";

    TCHAR buffer[MAX_PATH];
    SHGetSpecialFolderPath(nullptr, buffer, CSIDL_PROGRAM_FILES, FALSE);

    base_path = buffer;
    base_path /= "dotnet/host/fxr";
    auto search_paths = std::array {
      base_path 
    };
#else
    auto search_paths = std::array {
      std::filesystem::path("/usr/lib/dotnet/host/fxr"),
      std::filesystem::path("/usr/local/share/dotnet/host/fxr")
    };
#endif // !DOTOTHER_WINDOWS
    for (const auto& path : search_paths) {
      if (!std::filesystem::exists(path)) {
        continue;
      }

      for (const auto& dir : std::filesystem::recursive_directory_iterator(path)) {
        if (!dir.is_directory()) {
          continue;
        }

        auto dp = dir.path();
        if (dp.string().find(DOTOTHER_DOTNET_TARGET_VERSION_MAJOR_STR) == std::string::npos) {
          continue;
        }

        auto res = dp / DOTOTHER_HOSTFXR_NAME;
        if (!std::filesystem::exists(res)) {
          return std::nullopt;
        }

        return res;
      }
    }

    return std::nullopt;
  }

} // anonymous namespace

  bool Host::LoadClrFunctions() {
    std::optional<std::filesystem::path> host_path = GetHostPath();
    if (!host_path.has_value()) {
      util::print(DO_STR("Failed to get host path") , MessageLevel::CRITICAL);
      return false;
    }

#ifdef DOTOTHER_WINDOWS
    void* hostfxr_lib = LoadLibraryW(host_path->c_str());
    if (hostfxr_lib == nullptr) {
      util::print(DO_STR("Failed to load hostfxr") , MessageLevel::CRITICAL);
      return false;
    }
#else
    void* hostfxr_lib = dlopen(host_path->c_str(), RTLD_NOW);
    if (hostfxr_lib == nullptr) {
      util::print(DO_STR("Failed to load hostfxr") , MessageLevel::CRITICAL);
      return false;
    }
#endif

    coreclr.init_host_cmd_line = LoadFunction<hostfxr_initialize_for_dotnet_command_line_fn>(hostfxr_lib, "hostfxr_initialize_for_dotnet_command_line");
    coreclr.init_host_config = LoadFunction<hostfxr_initialize_for_runtime_config_fn>(hostfxr_lib, "hostfxr_initialize_for_runtime_config");
    coreclr.get_runtime_delegate = LoadFunction<hostfxr_get_runtime_delegate_fn>(hostfxr_lib, "hostfxr_get_runtime_delegate");
    // coreclr.run_app = LoadFunction<hostfxr_run_app_fn>(hostfxr_lib, "hostfxr_run_app");
    coreclr.close_host_fxr = LoadFunction<hostfxr_close_fn>(hostfxr_lib, "hostfxr_close");
    coreclr.set_error_writer = LoadFunction<hostfxr_set_error_writer_fn>(hostfxr_lib, "hostfxr_set_error_writer");
    
    if (coreclr.init_host_cmd_line == nullptr || coreclr.init_host_config == nullptr || 
        coreclr.get_runtime_delegate == nullptr || coreclr.close_host_fxr == nullptr) {
      util::print(DO_STR("Failed to load hostfxr functions") , MessageLevel::CRITICAL);
      return false;
    }
    util::print(DO_STR("Hostfxr functions loaded successfully") , MessageLevel::INFO);

    return coreclr.init_host_cmd_line != nullptr && coreclr.init_host_config != nullptr && 
          coreclr.get_runtime_delegate != nullptr && coreclr.close_host_fxr != nullptr;
  }

  bool Host::InitializeHost() {
    hostfxr_handle host_fxr = nullptr;
    int32_t rc = coreclr.init_host_config(config->host_config_path.c_str() , nullptr , &host_fxr);
    if (rc != 0 || host_fxr == nullptr) {
      util::print(DO_STR("Could not initialize host_fxr handle : {:#08x}"), MessageLevel::CRITICAL,  rc);
      coreclr.close_host_fxr(host_fxr);
      return false;
    }

    void* delegate = nullptr;
    rc = coreclr.get_runtime_delegate(host_fxr , hdt_load_assembly_and_get_function_pointer, &delegate);
    if (rc != 0 || delegate == nullptr) {
      util::print(DO_STR("Could not get managed function pointer : {:#08x}"), MessageLevel::CRITICAL,  rc);
      coreclr.close_host_fxr(host_fxr);
      return false;
    } 
    coreclr.get_managed_function_ptr = (load_assembly_and_get_function_pointer_fn)delegate;
    if (coreclr.get_managed_function_ptr == nullptr) {
      util::print(DO_STR("Failed to get managed function pointer"), MessageLevel::CRITICAL);
      return false;
    }
    util::print(DO_STR("Managed function pointer loaded") , MessageLevel::INFO);
    
    host_calls.entry = nullptr;
    host_calls.entry = LoadManagedFunction<EntryPoint>(config->dotnet_type.c_str(), config->entry_point.c_str());
    if (host_calls.entry == nullptr) {
      util::print(DO_STR("Failed to load entry point") , MessageLevel::CRITICAL);
      return 1;
    }
    util::print(DO_STR("Entry point loaded") , MessageLevel::INFO);

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
    interop.get_property_type = LoadManagedFunction<GetPropertyType>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetPropertyType"));
    interop.get_property_attributes = LoadManagedFunction<GetPropertyAttributes>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetPropertyAttributes"));

    interop.get_attr_value = LoadManagedFunction<GetAttributeValue>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetAttributeValue"));
    interop.get_attr_type = LoadManagedFunction<GetAttributeType>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetAttributeType"));

    interop.get_method_name = LoadManagedFunction<GetMethodName>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetMethodName"));
    interop.get_method_return_type = LoadManagedFunction<GetMethodReturnType>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetMethodReturnType"));
    interop.get_method_param_types = LoadManagedFunction<GetMethodParameterTypes>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetMethodParameterTypes"));
    interop.get_method_attributes = LoadManagedFunction<GetMethodAttributes>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetMethodAttributes"));
    interop.get_method_accessibility = LoadManagedFunction<GetMethodAccessibility>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetMethodAccessibility"));

    interop.set_internal_calls = LoadManagedFunction<SetInternalCalls>(DO_STR("DotOther.Managed.Interop.InternalCallManager, DotOther.Managed"), DO_STR("SetInternalCalls"));
    interop.set_internal_call = LoadManagedFunction<SetInternalCall>(DO_STR("DotOther.Managed.Interop.InternalCallManager, DotOther.Managed"), DO_STR("SetInternalCall"));

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
      util::print(DO_STR("Failed to load managed functions, runtime not bound to assembly") , MessageLevel::CRITICAL);
      throw std::runtime_error("Failed to load managed functions, runtime not bound to assembly");
    }
  }

  void* Host::LoadManagedFunction(const std::filesystem::path& asm_path, const dostring& type_name, const dostring& method_name, const dochar* delegate_type) const {
    if (!config.has_value()) {
      assert(false && "Host configuration not set!");
    }

    if (!std::filesystem::exists(asm_path)) {
      using namespace std::string_view_literals;
      util::print(DO_STR("Assembly {} does not exist!"sv), MessageLevel::ERR , asm_path.string());
      return nullptr;
    }

    void* handle = nullptr;
    int32_t rc = coreclr.get_managed_function_ptr(asm_path.c_str(), type_name.c_str(), method_name.c_str(), delegate_type, nullptr, &handle);
    if (rc == -1 || handle == nullptr) {
      util::print(DO_STR("Failed to load managed function {} ({}) | error code : {}"), MessageLevel::ERR,
#ifdef DOTOTHER_WIDE_CHARS
        util::WideToChar(method_name), util::WideToChar(type_name),
#else
        method_name, type_name,
#endif    
        rc
      );
      return nullptr;
    } else {
      util::print(DO_STR("Managed function {} ({}) loaded successfully"), MessageLevel::INFO ,
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
