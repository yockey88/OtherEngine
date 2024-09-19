/**
 * \file host.hpp 
 **/
#ifndef DOTOTHER_NATIVE_HOST_HPP
#define DOTOTHER_NATIVE_HOST_HPP

#include <filesystem>
#include <optional>

#include "hostfxr.h"

#include "defines.hpp"
#include "native_string.hpp"
#include "assembly.hpp"

namespace dotother {

	using exception_callback_t = void(*)(const NString message);
	using log_callback_t = void(*)(const NString message, MessageLevel level);
  
  struct DotOtherArgs {
    exception_callback_t exception_callback = nullptr;
    log_callback_t log_callback = nullptr;
  };

  using EntryPoint = void(DOTOTHER_CALLTYPE*)(DotOtherArgs args);

  struct HostConfig {
    std::filesystem::path host_config_path;
		std::filesystem::path managed_asm_path;

		dostring_t dotnet_type;
		dostring_t entry_point;
		const dochar* delegate_type = DOTNET_UNMANAGED_FUNCTION;

		exception_callback_t exception_callback = nullptr;
		log_callback_t log_callback = nullptr;
  };

  class Host {
    public:
      Host(const HostConfig& config) 
        : config(config) {}
      ~Host() {}
      
      bool LoadHost();
      void CallEntryPoint(/* const DotOtherArgs */);
      void UnloadHost();

      AssemblyContext CreateAsmContext(const std::string_view);
      void UnloadAssemblyContext(AssemblyContext& InLoadContext);

    private:
      hostfxr_handle host_fxr = nullptr;

      struct HostCalls {
        EntryPoint entry = nullptr;
        // Add more function pointers here
      } host_calls;

      std::filesystem::path managed_asm;

      bool is_loaded = false;

      HostConfig config;

      bool LoadClrFunctions();
      bool InitializeHost();
      void LoadManagedFunctions();

      std::optional<std::filesystem::path> GetHostPath();

      void* LoadManagedFunction(const std::filesystem::path& asm_path, const dostring_t& typ_name, const dostring_t& method_name, const dochar* delegate_type = DOTNET_UNMANAGED_FUNCTION) const;

      template <typename Fn>
      Fn LoadManagedFunction(const dostring_t& type_name, const dostring_t& method_name , const dochar* delegate_type = DOTNET_UNMANAGED_FUNCTION) const {
        return (Fn)(LoadManagedFunction(config.managed_asm_path, type_name, method_name, delegate_type));
      }
  };

} // namespace dotother

#endif // !DOTOTHER_NATIVE_HOST_HPP
