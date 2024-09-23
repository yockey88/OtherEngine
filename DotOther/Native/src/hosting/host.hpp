/**
 * \file hosting/host.hpp 
 **/
#ifndef DOTOTHER_NATIVE_HOST_HPP
#define DOTOTHER_NATIVE_HOST_HPP

#include <filesystem>
#include <optional>
#include <string_view>

#include "coreclr_delegates.h"
#include "hostfxr.h"

#include "core/defines.hpp"
#include "core/hook_definitions.hpp"
#include "hosting/interop_interface.hpp"
#include "hosting/assembly.hpp"

namespace dotother {
  
  struct DotOtherArgs {
    exception_callback_t exception_callback = nullptr;
    log_callback_t log_callback = nullptr;
    invoke_native_method_hook_t invoke_native_method_hook = nullptr;
  };

  using EntryPoint = void(DOTOTHER_CALLTYPE*)(DotOtherArgs args);

  struct HostConfig {
    std::filesystem::path host_config_path;
    std::filesystem::path managed_asm_path;
    
    dostring dotnet_type;
    dostring entry_point;
    const dochar* delegate_type = DOTNET_UNMANAGED_FUNCTION;

    MessageLevel log_level = MessageLevel::MESSAGE;

    /// Callbacks for managed code
    exception_callback_t exception_callback = nullptr;
    log_callback_t log_callback = nullptr;
    invoke_native_method_hook_t invoke_native_method_hook = nullptr;

    /// native host engine configuration
    coreclr_error_callback_t coreclr_error_callback = nullptr;
    internal_logging_hook_t internal_logging_hook = nullptr;
  };

  class Host {
    public:
      static Host* Instance();
      static Host* Instance(const HostConfig& config);
      static void Destroy();

      bool LoadHost();
      void CallEntryPoint(/* const DotOtherArgs */);
      void UnloadHost();

      AssemblyContext CreateAsmContext(const std::string_view);
      void UnloadAssemblyContext(AssemblyContext& InLoadContext);

      interface_bindings::FunctionTable& GetInteropInterface();

    private:
      Host(const HostConfig& config) 
          : config(config) {}
      ~Host() {}

      Host(Host&&) = delete;
      Host(const Host&) = delete;
      Host& operator=(Host&&) = delete;
      Host& operator=(const Host&) = delete;

      static Host* instance;
      hostfxr_handle host_fxr = nullptr;

      struct HostCalls {
        EntryPoint entry = nullptr;
        // Add more function pointers here
      } host_calls;

      std::filesystem::path managed_asm;

      bool is_loaded = false;

      std::optional<HostConfig> config;

      bool LoadClrFunctions();
      bool InitializeHost();
      void LoadManagedFunctions();

      std::optional<std::filesystem::path> GetHostPath();

      void* LoadManagedFunction(const std::filesystem::path& asm_path, const dostring& typ_name, const dostring& method_name, 
                                const dochar* delegate_type = DOTNET_UNMANAGED_FUNCTION) const;

      template <typename Fn>
      Fn LoadManagedFunction(const dostring& type_name, const dostring& method_name , const dochar* delegate_type = DOTNET_UNMANAGED_FUNCTION) const {
        if (!config.has_value()) {
          assert(false && "Host configuration not set!");
        }

        return (Fn)(LoadManagedFunction(config->managed_asm_path, type_name, method_name, delegate_type));
      }
  };

} // namespace dotother

#endif // !DOTOTHER_NATIVE_HOST_HPP
