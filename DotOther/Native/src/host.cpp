/**
  * \file host.cpp
  */
#include "host.hpp"

#include <cstdint>
#include <optional>
#include <ostream>

#include <string>
#include <filesystem> 
#include <iostream>

#include <spdlog/fmt/fmt.h>
#include <wincrypt.h>

#include "assembly.hpp"
#include "coreclr_delegates.h"

#include "defines.hpp"
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

  coreclr.set_error_writer([](const dochar* message) {
    dostring_view message_view(message);
    int len = WideCharToMultiByte(CP_UTF8, 0, message, static_cast<int>(message_view.length()), nullptr, 0, nullptr, nullptr);
		std::string result(len, 0);
		(void)WideCharToMultiByte(CP_UTF8, 0, message, static_cast<int>(message_view.length()), result.data(), len, nullptr, nullptr);
    util::print(DO_STR("CoreCLR error: {}"), result);
  });

  is_loaded = InitializeHost();
  if (!is_loaded) {
    util::print(DO_STR("Failed to initialize host, unloading..."));
    UnloadHost();
    return false;
  }

  util::print(DO_STR("Host loaded successfully"));
  return true;
}

void Host::CallEntryPoint(/* const DotOtherArgs */) {
  if (!is_loaded) {
    std::cout << "Host is not loaded" << std::endl;
    return;
  }

  if (host_calls.entry == nullptr) {
    std::cout << "Entry point is null" << std::endl;
    return;
  }  

  LoadManagedFunctions();

  DotOtherArgs args = {
    .exception_callback = config.exception_callback,
    .log_callback = config.log_callback,
  };
  host_calls.entry(args);
}

void Host::UnloadHost() {
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
  NScopedString ctx_name = NString::New(name);
  AssemblyContext ctx ;
  ctx.context_id = interop::create_assembly_load_context(ctx_name);
  ctx.host = this;
  return ctx;
}

void Host::UnloadAssemblyContext(AssemblyContext& InLoadContext) {
  interop::unload_assembly_load_context(InLoadContext.context_id);
  InLoadContext.context_id = -1;
  InLoadContext.assemblies.clear();
}


bool Host::LoadClrFunctions() {
  std::optional<std::filesystem::path> host_path = GetHostPath();
  if (!host_path.has_value()) {
    std::cout << "Failed to get host path" << std::endl;
    return false;
  }

  void* hostfxr_lib = LoadLibraryW(host_path->c_str());
  if (hostfxr_lib == nullptr) {
    std::cout << "Failed to load hostfxr" << std::endl;
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
    std::cout << "Failed to load hostfxr functions" << std::endl;
    return false;
  }
  std::cout << "Hostfxr functions loaded successfully\n";

  return coreclr.init_host_cmd_line != nullptr && coreclr.init_host_config != nullptr && 
         coreclr.get_runtime_delegate != nullptr && coreclr.close_host_fxr != nullptr;
}

bool Host::InitializeHost() {
  hostfxr_handle host_fxr = nullptr;
  int32_t rc = coreclr.init_host_config(config.host_config_path.c_str() , nullptr , &host_fxr);
  if (rc != 0 || host_fxr == nullptr) {
    std::cerr << "Could not initialize host_fxr handle : " << std::hex << std::showbase << rc << "!\n";
    coreclr.close_host_fxr(host_fxr);
    return false;
  }

  void* delegate = nullptr;
  rc = coreclr.get_runtime_delegate(host_fxr , hdt_load_assembly_and_get_function_pointer, &delegate);
  if (rc != 0 || delegate == nullptr) {
    std::cerr << "Could not initialize host_fxr handle : " << std::hex << std::showbase << rc << "!\n";
    coreclr.close_host_fxr(host_fxr);
    return false;
  } 
  coreclr.get_managed_function_ptr = (load_assembly_and_get_function_pointer_fn)delegate;
  if (coreclr.get_managed_function_ptr == nullptr) {
    std::cerr << "Could not get managed function pointer\n";
    return false;
  }
  std::cout << "assembly loaded successfully\n";
  
  host_calls.entry = nullptr;
  host_calls.entry = LoadManagedFunction<EntryPoint>(config.dotnet_type.c_str(), config.entry_point.c_str());
  if (host_calls.entry == nullptr) {
    std::cout << "Failed to load managed function : error code : " << std::hex << std::showbase << rc << std::endl;
    return 1;
  }
  std::cout << "entry point loaded\n";

  return true;
}

void Host::LoadManagedFunctions() {
  interop::create_assembly_load_context = LoadManagedFunction<CreateAssemblyLoadContext>(DO_STR("DotOther.Managed.AssemblyLoader, DotOther.Managed"), DO_STR("CreateAssemblyLoadContext"));
  interop::unload_assembly_load_context = LoadManagedFunction<UnloadAssemblyLoadContext>(DO_STR("DotOther.Managed.AssemblyLoader, DotOther.Managed"), DO_STR("UnloadAssemblyLoadContext"));
  interop::get_last_load_status = LoadManagedFunction<GetLastLoadStatus>(DO_STR("DotOther.Managed.AssemblyLoader, DotOther.Managed"), DO_STR("GetLastLoadStatus"));
  interop::load_managed_assembly = LoadManagedFunction<LoadManagedAssembly>(DO_STR("DotOther.Managed.AssemblyLoader, DotOther.Managed"), DO_STR("LoadManagedAssembly"));
  interop::get_assembly_name = LoadManagedFunction<GetAssemblyName>(DO_STR("DotOther.Managed.AssemblyLoader, DotOther.Managed"), DO_STR("GetAssemblyName"));

  interop::get_asm_types = LoadManagedFunction<GetAsmTypes>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetAsmTypes"));
  interop::get_type_id = LoadManagedFunction<GetTypeId>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetTypeId"));
  interop::get_full_type_name = LoadManagedFunction<GetFullTypeName>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetFullTypeName"));
  interop::get_asm_qualified_name = LoadManagedFunction<GetAsmQualifiedName>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetAsmQualifiedName"));
  interop::get_base_type = LoadManagedFunction<GetBaseType>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetBaseType"));
  interop::get_type_size = LoadManagedFunction<GetTypeSize>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetTypeSize"));
  interop::is_type_subclass_of = LoadManagedFunction<IsTypeSubclassOf>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("IsTypeDerivedFrom"));
  interop::is_type_assignable_to = LoadManagedFunction<IsTypeAssignableTo>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("IsAssignableTo"));
  interop::is_type_assignable_from = LoadManagedFunction<IsTypeAssignableFrom>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("IsAssignableFrom"));
  interop::is_type_sz_array = LoadManagedFunction<IsTypeSZArray>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("IsSzArray"));
  interop::get_element_type = LoadManagedFunction<GetElementType>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetElementType"));
  interop::get_type_methods = LoadManagedFunction<GetTypeMethods>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetTypeMethods"));
  interop::get_type_fields = LoadManagedFunction<GetTypeFields>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetTypeFields"));
  interop::get_type_properties = LoadManagedFunction<GetTypeProperties>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetTypeProperties"));
  interop::has_type_attribute = LoadManagedFunction<HasTypeAttribute>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("HasAttribute"));
  interop::get_type_attributes = LoadManagedFunction<GetTypeAttributes>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetAttributes"));
  interop::get_type_managed_type = LoadManagedFunction<GetTypeManagedType>(DO_STR("DotOther.Managed.InteropInterface, DotOther.Managed"), DO_STR("GetTypeManagedType"));

  interop::collect_garbage = LoadManagedFunction<CollectGarbage>(DO_STR("DotOther.Managed.GarbageCollector, DotOther.Managed"), DO_STR("CollectGarbage"));
  interop::wait_for_pending_finalizers = LoadManagedFunction<WaitForPendingFinalizers>(DO_STR("DotOther.Managed.GarbageCollector, DotOther.Managed"), DO_STR("WaitForPendingFinalizers"));
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
    std::cout << "Failed to get hostfxr path : " << rc << std::endl;
    return std::nullopt;
  }

  std::filesystem::path host_path = hostfxr_path;
  if (!std::filesystem::exists(host_path)) {
    std::cout << "Hostfxr path does not exist" << std::endl;
    return std::nullopt;
  }

  return host_path;
}

void* Host::LoadManagedFunction(const std::filesystem::path& asm_path, const dostring_t& type_name, const dostring_t& method_name, const dochar* delegate_type) const {
  if (!std::filesystem::exists(asm_path)) {
    using namespace std::string_view_literals;
    std::string msg = fmt::format("Assembly {} does not exist!"sv, asm_path.string());
    std::cout << msg << std::endl;
  }

  void* handle = nullptr;
  int32_t rc = coreclr.get_managed_function_ptr(asm_path.c_str(), type_name.c_str(), method_name.c_str(), delegate_type, nullptr, &handle);
  if (handle == nullptr) {
    std::cout << "Failed to load managed function : handle is null | error code : " << std::hex << rc << std::dec << std::endl;
    return nullptr;
  }

  return handle;
}

} // namespace dotother
