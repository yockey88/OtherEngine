/**
  * \file host.cpp
  */
#include "host.hpp"

#include <cstdint>
#include <optional>
#include <ostream>

#include <string>
#include <string_view>
#include <filesystem> 
#include <iostream>

#include <spdlog/fmt/fmt.h>
#include <wincrypt.h>

#include "coreclr_delegates.h"

#include "defines.hpp"
#include "interop_interface.hpp"
#include "native_string.hpp"

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

#ifdef _WIN32
template <typename Fn>
Fn LoadFunction(void* handle, const char* name) {
  auto fn = (Fn)GetProcAddress((HMODULE)handle, name);
  if (fn == nullptr) {
    std::cout << "Failed to load function : " << name << std::endl;
    return nullptr;
  }
  return fn;
}
#else // __linux__
#error "LoadFunction Not implemented on this platform"
/// auto fn = (Fn)dlsym(handle, name);
#endif // !_WIN32

bool Host::LoadHost() {
  if (is_loaded) {
    return true;
  }

  if (!std::filesystem::exists(config.host_config_path)) {
    std::cout << "Host config path does not exist" << std::endl;
    return false;
  }

  if (!std::filesystem::exists(config.managed_asm_path)) {
    std::cout << "Managed assembly path does not exist" << std::endl;
    return false;
  }

  if (config.log_callback == nullptr) {
    std::cout << "Log callback is null" << std::endl;
    return false;
  }

  if (config.exception_callback == nullptr) {
    std::cout << "Exception callback is null" << std::endl;
    return false;
  }

  is_loaded = LoadClrFunctions();
  if (!is_loaded) {
    return false;
  }

  coreclr.set_error_writer([](const wchar_t* message) {
    dostring_view message_view(message);
    int len = WideCharToMultiByte(CP_UTF8, 0, message, static_cast<int>(message_view.length()), nullptr, 0, nullptr, nullptr);

		std::string result(len, 0);
		(void)WideCharToMultiByte(CP_UTF8, 0, message, static_cast<int>(message_view.length()), result.data(), len, nullptr, nullptr);

    std::cout << "Hostfxr error : " << result << std::endl;
  });

  is_loaded = InitializeHost();
  return is_loaded;
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
    return 1;
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
    return false;
  } 
  coreclr.close_host_fxr(host_fxr);
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
  interop::function_table.create_assembly_load_context = LoadManagedFunction<CreateAssemblyLoadContextFn>(DO_STR("DotOther.Managed.AssemblyLoader"), DO_STR("UnloadAssemblyLoadContext"));
  interop::function_table.unload_assembly_load_context = LoadManagedFunction<UnloadAssemblyLoadContextFn>(DO_STR("DotOther.Managed.AssemblyLoader"), DO_STR("UnloadAssemblyLoadContext"));
  // interop::function_table.set_internal_calls(nullptr, 0);
  // interop::function_table.load_managed_assembly(0, nullptr);
  // interop::function_table.get_assembly_name(0);
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
    throw std::runtime_error("Assembly path does not exist");
  }

  void* handle = nullptr;
  int32_t rc = coreclr.get_managed_function_ptr(asm_path.c_str(), type_name.c_str(), method_name.c_str(), delegate_type, nullptr, &handle);
  if (handle == nullptr) {
    std::cout << "Failed to load managed function : handle is null | error code : " << rc << std::endl;
    return nullptr;
  }

  return handle;
}

} // namespace dotother
