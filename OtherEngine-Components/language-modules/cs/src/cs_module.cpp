/**
 * \file cs_module.cpp
*/
#include "cs_module.hpp"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/threads.h>

#include "core/filesystem.hpp"
#include "cs_script.hpp"

namespace other {

  bool CsModule::Initialize() {
    LogDebug("Initializing Mono Runtime");

    mono_path = Filesystem::FindEngineCoreDir(kMonoPath);
    if (mono_path.empty()) {
      LogError("Mono .NET runtime assemblies not found");
      return load_success;
    }

    mono_config_path = Filesystem::FindEngineCoreDir(kMonoConfigPath);
    if (mono_config_path.empty()) {
      LogError("Mono .NET config file not found");
      return load_success;
    }

    LogDebug("Mono Path: {}" , mono_path.string());
    mono_set_dirs(mono_path.string().data() , mono_config_path.string().data());
    root_domain = mono_jit_init(kRootDomainName.data());

    char* app_domain_name = const_cast<char*>(kAppDomainName.data());
    app_domain = mono_domain_create_appdomain(app_domain_name , nullptr);

    mono_thread_set_main(mono_thread_current());
    mono_domain_set(app_domain , true);

    LogDebug("Mono Runtime initialized");

    std::filesystem::path mono_core = std::filesystem::absolute(kEngineCoreDir) / "bin" / "Debug" / "OtherEngine-CsCore" / "OtherEngine-CsCore.dll";
    if (!Filesystem::FileExists(mono_core.string())) {
      LogError("Mono core assembly {} not found, unloading C# module" , mono_core.string());

      mono_domain_set(root_domain , true);
      mono_domain_unload(app_domain);
      mono_jit_cleanup(root_domain);
      return load_success;
    }

    LoadScriptModule(ScriptModuleInfo {
      .name = "C#-Core" ,
      .paths = { 
        mono_core.string()
      }
    });

    load_success = true;
    return load_success;
  }

  bool CsModule::Reinitialize() {
    LogDebug("Reinitializing Mono Runtime");
    if (!load_success) {
      return Initialize();
    }

    for (auto& [id , module] : loaded_modules) {
      module->Shutdown();
    }

    mono_domain_set(root_domain , true);
    mono_domain_unload(app_domain);
    mono_jit_cleanup(root_domain);

    load_success = false;

    root_domain = mono_jit_init(kRootDomainName.data());

    char* app_domain_name = const_cast<char*>(kAppDomainName.data());
    app_domain = mono_domain_create_appdomain(app_domain_name , nullptr);

    mono_thread_set_main(mono_thread_current());
    mono_domain_set(app_domain , true);

    for (auto& [id , module] : loaded_modules) {
      module->Initialize();
    }

    LogDebug("Mono Runtime reinitialized");
    load_success = true;
    return load_success;
  }

  void CsModule::Shutdown() {
    if (!load_success) {
      return;
    }

    for (auto& [id , module] : loaded_modules) {
      module->Shutdown();
      delete module;
    }
    loaded_modules.clear();

    mono_domain_set(root_domain , true);
    mono_domain_unload(app_domain);
    mono_jit_cleanup(root_domain);

    app_domain = nullptr;
    root_domain = nullptr;

    LogDebug("Mono Runtime shutdown");
  }
  
  ScriptModule* CsModule::GetScriptModule(const std::string& name) {
    if (!load_success) {
      LogWarn("Attempting to get script module {} when C# module is not loaded" , name);
      return nullptr;
    }

    auto hash = FNV(name);
    auto* module = GetScriptModule(hash);
    if (module == nullptr) {
      LogError("Script module {} not found" , name);
    }

    return module;
  }
  
  ScriptModule* CsModule::GetScriptModule(const UUID& id) {
    if (!load_success) {
      LogWarn("Attempting to get script module {} when C# module is not loaded" , id);
      return nullptr;
    }

    if (loaded_modules.find(id) != loaded_modules.end()) {
      return loaded_modules[id];
    }

    LogError("Script module {} not found" , id);
    return nullptr;
  }

  ScriptModule* CsModule::LoadScriptModule(const ScriptModuleInfo& module_info) {
    UUID id = FNV(module_info.name);
    if (loaded_modules.find(id) != loaded_modules.end()) {
      LogError("Script module {} already loaded" , module_info.name);
      return loaded_modules[id];
    } else {
      LogDebug("Loading script module {} from {}" , module_info.name , module_info.paths[0]);
    }

    loaded_modules[id] = new CsScript(GetEngine() , root_domain , app_domain , module_info.paths[0]);
    loaded_modules[id]->Initialize();
    return loaded_modules[id];
  }
      
  void CsModule::UnloadScriptModule(const std::string& name) {
    UUID id = FNV(name);
    if (loaded_modules.find(id) == loaded_modules.end()) {
      LogError("Attempting to unload cript module {} that is not loaded" , name);
      return;
    }

    loaded_modules[id]->Shutdown();
    delete loaded_modules[id];
    loaded_modules.erase(id);
  }

} // namespace other

other::Plugin* CreatePlugin(other::Engine* engine) {
  return new other::CsModule(engine);
}

void DestroyPlugin(other::Plugin* module) {
  delete module;
}
