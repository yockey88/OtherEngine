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

  void CsModule::Initialize() {
    LogDebug("Initializing Mono Runtime");

    if (!Filesystem::FileExists(std::string{ kMonoPath })) {
      LogError("Mono external directory not found: {}" , kMonoPath);
      return;
    }

    mono_set_dirs(kMonoPath.data() , kMonoPath.data());
    root_domain = mono_jit_init(kRootDomainName.data());

    char* app_domain_name = const_cast<char*>(kAppDomainName.data());
    app_domain = mono_domain_create_appdomain(app_domain_name , nullptr);

    mono_thread_set_main(mono_thread_current());
    mono_domain_set(app_domain , true);

    LogDebug("Mono Runtime initialized");

    LoadScriptModule(ScriptModuleInfo {
      .name = "C#-Core" ,
      .paths = { 
        "bin/Debug/OtherEngine-ScriptCore/OtherEngine-ScriptCore.dll" 
      }
    });
  }

  void CsModule::Reinitialize() {
    LogDebug("Reinitializing Mono Runtime");

    for (auto& [id , module] : loaded_modules) {
      module->Shutdown();
    }

    mono_domain_set(root_domain , true);
    mono_domain_unload(app_domain);

    char* app_domain_name = const_cast<char*>(kAppDomainName.data());
    app_domain = mono_domain_create_appdomain(app_domain_name , nullptr);

    mono_thread_set_main(mono_thread_current());
    mono_domain_set(app_domain , true);

    for (auto& [id , module] : loaded_modules) {
      module->Initialize();
    }
  }

  void CsModule::Shutdown() {
    LogDebug("Shutting down Mono Runtime");

    auto it = loaded_modules.begin();
    while (it != loaded_modules.end()) {
      it->second->Shutdown();
      delete it->second;
      it = loaded_modules.erase(it);
    }

    mono_domain_set(root_domain , true);
    mono_domain_unload(app_domain);
    mono_jit_cleanup(root_domain);

    LogDebug("Mono Runtime shutdown");
  }
  
  ScriptModule* CsModule::GetScriptModule(const std::string& name) {
    auto hash = FNV(name);
    auto* module = GetScriptModule(hash);
    if (module == nullptr) {
      LogError("Script module {} not found" , name);
    }

    return module;
  }
  
  ScriptModule* CsModule::GetScriptModule(const UUID& id) {
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
