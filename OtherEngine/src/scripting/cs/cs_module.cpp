/**
 * \file scripting/cs/cs_module.cpp
*/
#include "scripting/cs/cs_module.hpp"

#include <core/dotother_defines.hpp>
#include <filesystem>

#include <hosting/assembly.hpp>
#include <hosting/host.hpp>

#include "core/filesystem.hpp"
#include "core/platform.hpp"
#include "application/app_state.hpp"
#include "scripting/cs/cs_script.hpp"

namespace other {
namespace {

  static dotother::Host* host = nullptr;

  struct DotOtherAssemblyContexts {
    dotother::AssemblyContext core_ctx;

    std::map<int32_t , std::vector<dotother::ref<dotother::Assembly>>> assemblies;
  };

  static DotOtherAssemblyContexts assembly_contexts;

} // anonymous namespace

  using namespace std::string_view_literals;

  bool CsModule::Initialize() {
    try {
      const dotother::HostConfig config = {
        /// TODO: create dotother config folder to store paths and type/entry point info
        .host_config_path = DO_STR("./DotOther/Managed/DotOther.Managed.runtimeconfig.json") ,
        .managed_asm_path = DO_STR("./bin/Debug/DotOther.Managed/net8.0/DotOther.Managed.dll") ,
        .dotnet_type = DO_STR("DotOther.Managed.DotOtherHost, DotOther.Managed") ,
        .entry_point = DO_STR("EntryPoint") ,
        .log_level = dotother::MessageLevel::TRACE,

        .exception_callback = [](const dotother::NString message) {
          std::string  msg = message;
          OE_ERROR("C# Exception Caught : \n\t{}" , msg);
        } ,
        .log_callback = [](const dotother::NString message, dotother::MessageLevel level) {
          switch (level) {
            case dotother::MessageLevel::TRACE:
              OE_TRACE("{}" , message);
            break;
            case dotother::MessageLevel::DEBUG:
              OE_DEBUG("{}" , message);
            break;
            case dotother::MessageLevel::MESSAGE:
            case dotother::MessageLevel::INFO:
              OE_INFO("{}" , message);
            break;
            case dotother::MessageLevel::WARNING:
              OE_WARN("{}" , message);
            break;
            case dotother::MessageLevel::ERR:
              OE_ERROR("{}" , message);
            break;
            case dotother::MessageLevel::CRITICAL:
              OE_CRITICAL("{}" , message);
            break;

          }
        } ,      
        /// TODO: decide if DotOther should be responsible for invoking native methods or if it should be the responsibility of the user
        .invoke_native_method_hook = [](uint64_t object_handle, const dotother::NString method_name) {
          std::string mname = method_name;
          OE_DEBUG("Invoking Native Method on object {:#08x}" , object_handle);
          OE_DEBUG(" > Method Name: {}" , mname);
          dotother::InteropInterface::Instance().InvokeNativeFunction(object_handle, mname);
        },
        .internal_logging_hook = [](const std::string_view message, dotother::MessageLevel level, bool verbose) {
          switch (level) {
            case dotother::MessageLevel::MESSAGE:
            case dotother::MessageLevel::TRACE:
              OE_TRACE("{}" , message);
            break;
            case dotother::MessageLevel::DEBUG:
              OE_DEBUG("{}" , message);
            break;
            case dotother::MessageLevel::INFO:
              OE_INFO("{}" , message);
            break;
            case dotother::MessageLevel::WARNING:
              OE_WARN("{}" , message);
            break;
            case dotother::MessageLevel::ERR:
              OE_ERROR("{}" , message);
            break;
            case dotother::MessageLevel::CRITICAL:
              OE_CRITICAL("{}" , message);
            break;

          }
        }
      };

      host = dotother::Host::Instance(config);
      if (!host->LoadHost()) {
        OE_ERROR("Failed to load C# host");
        return false;
      } else {
        OE_DEBUG("Loaded C# Host");
      }

      /// TODO: should clients be responsible for this????
      host->CallEntryPoint();

      assembly_contexts.core_ctx = host->CreateAsmContext("OtherEngine.CsCore");
      if (assembly_contexts.core_ctx.context_id == -1) {
        OE_ERROR("Failed to create C# core assembly context");
        return false;
      }

      auto& asms = assembly_contexts.assemblies[assembly_contexts.core_ctx.context_id] = {};
      asms.push_back(assembly_contexts.core_ctx.LoadAssembly("./bin/Debug/OtherEngine-CsCore/net8.0/OtherEngine-CsCore.dll"));

      load_success = true;
      return true;
    } catch (const std::exception& e) {
      OE_ERROR("Failed to create C# host : {}" , e.what());
      return false;
    }
  }

  void CsModule::Shutdown() {
    if (host == nullptr) {
      OE_WARN("Attempting to shutdown C# module when it is not loaded");
      return;
    }

    host->UnloadHost();
    host = nullptr;
    dotother::Host::Destroy();
  }

  void CsModule::Reload() {
    // auto proj = AppState::ProjectContext();
    // auto script_file = proj->GetMetadata().cs_project_file;

    // for (auto& [id, module] : loaded_modules) {
    //   module->Shutdown(); 
    //   delete module;
    // }
    // loaded_modules.clear();
    
    // OE_DEBUG("Kicking off build for scripts {}" , script_file);

    // if (!PlatformLayer::BuildProject(script_file)) {
    //   OE_ERROR("Failed to rebuild project scripts");
    // }
    
    // auto editor_file = proj->GetMetadata().cs_editor_project_file;

    // OE_DEBUG("Kicking off build for scripts {}" , editor_file);

    // if (!PlatformLayer::BuildProject(editor_file)) {
    //   OE_ERROR("Failed to rebuild editor scripts");
    // }
    
    // for (const auto& [id , module_info] : loaded_modules_data) {
    //   LoadScript(module_info);
    // }
  }
      
  ScriptModule* CsModule::GetScript(const std::string& name) {
    if (!load_success) {
      OE_WARN("Attempting to get script module {} when C# module is not loaded" , name);
      return nullptr;
    }

    std::string case_insensitive_name;
    std::transform(name.begin() , name.end() , std::back_inserter(case_insensitive_name) , ::toupper);

    auto hash = FNV(case_insensitive_name);
    OE_DEBUG("Looking for script module {} [{}]" , name , hash);

    auto* module = GetScript(hash);
    if (module == nullptr) {
      OE_ERROR("Script module {} not found" , name);
    }

    return module;
  }
  
  ScriptModule* CsModule::GetScript(const UUID& id) {
    if (!load_success) {
      OE_WARN("Attempting to get script module {} when C# module is not loaded" , id);
      return nullptr;
    }

    if (loaded_modules.find(id) != loaded_modules.end()) {
      return loaded_modules[id].Raw();
    }

    // OE_ERROR("Script module w/ ID [{}] not found" , id.Get());
    return nullptr;
  }

  ScriptModule* CsModule::LoadScript(const ScriptMetadata& module_info) {    
    std::string case_insensitive_name;
    std::transform(module_info.name.begin() , module_info.name.end() , std::back_inserter(case_insensitive_name) , ::toupper);

    UUID id = FNV(case_insensitive_name);
    OE_DEBUG("Loading C# script module {} [{}]" , module_info.name , id);

    if (loaded_modules.find(id) != loaded_modules.end()) {
      OE_WARN("  > Script module {} already loaded" , module_info.name);
      return loaded_modules[id].Raw();
    } 

    Path module_path = module_info.paths[0];
    std::string mod_path_str = module_path.filename().string();
    std::string mod_name = mod_path_str.substr(0 , mod_path_str.find_last_of('.'));
    loaded_modules[id] = NewRef<CsScript>(mod_name);
    loaded_modules[id]->Initialize();

    loaded_modules_data[id] = module_info;

    return loaded_modules[id].Raw();
  }
      
  void CsModule::UnloadScript(const std::string& name) {
    std::string case_insensitive_name;
    std::transform(name.begin() , name.end() , std::back_inserter(case_insensitive_name) , ::toupper);

    UUID id = FNV(case_insensitive_name);
    OE_DEBUG("Unloading C# script module {} [{}]" , name , id);

    if (loaded_modules.find(id) == loaded_modules.end()) {
      OE_ERROR("Attempting to unload script module {} that is not loaded" , name);
      return;
    }

    loaded_modules[id]->Shutdown();
    loaded_modules.erase(id);
  }
      
} // namespace other
