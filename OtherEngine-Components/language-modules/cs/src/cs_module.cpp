/**
 * \file cs_module.cpp
*/
#include "cs_module.hpp"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/threads.h>

#include "core/filesystem.hpp"
#include "cs_script.hpp"
#include "script_cache.hpp"

namespace other {

  bool CsModule::Initialize() {
    OE_DEBUG("Initializing Mono Runtime");

    mono_path = Filesystem::FindEngineCoreDir(kMonoPath);
    if (mono_path.empty()) {
      OE_ERROR("Mono .NET runtime assemblies not found");
      return load_success;
    }

    mono_config_path = Filesystem::FindEngineCoreDir(kMonoConfigPath);
    if (mono_config_path.empty()) {
      OE_ERROR("Mono .NET config file not found");
      return load_success;
    }

    OE_DEBUG("Mono Path: {}" , mono_path.string());
    mono_set_dirs(mono_path.string().data() , mono_config_path.string().data());
    root_domain = mono_jit_init(kRootDomainName.data());

    char* app_domain_name = const_cast<char*>(kAppDomainName.data());
    app_domain = mono_domain_create_appdomain(app_domain_name , nullptr);

    mono_thread_set_main(mono_thread_current());
    mono_domain_set(app_domain , true);

    OE_DEBUG("Mono Runtime initialized");

    std::filesystem::path mono_core = std::filesystem::absolute(kEngineCoreDir) / "bin" / "Debug" / "OtherEngine-CsCore" / "OtherEngine-CsCore.dll";
    if (!Filesystem::FileExists(mono_core.string())) {
      OE_ERROR("Mono core assembly {} not found, unloading C# module" , mono_core.string());

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

    CsScriptCache::InitializeCache();

    load_success = true;
    return load_success;
  }

  bool CsModule::Reinitialize() {
    OE_DEBUG("Reinitializing Mono Runtime");
    if (!load_success) {
      return Initialize();
    }

    CsScriptCache::ShutdownCache();

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

    CsScriptCache::InitializeCache();

    OE_DEBUG("Mono Runtime reinitialized");
    load_success = true;
    return load_success;
  }

  void CsModule::Shutdown() {
    if (!load_success) {
      return;
    }

    CsScriptCache::ShutdownCache();

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

    OE_DEBUG("Mono Runtime shutdown");
  }
  
  ScriptModule* CsModule::GetScriptModule(const std::string& name) {
    if (!load_success) {
      OE_WARN("Attempting to get script module {} when C# module is not loaded" , name);
      return nullptr;
    }

    for (const auto& m : loaded_modules) {
      OE_DEBUG("  {}" , m.first);
    }

    std::string case_insensitive_name;
    std::transform(name.begin() , name.end() , std::back_inserter(case_insensitive_name) , ::toupper);

    auto hash = FNV(case_insensitive_name);
    OE_DEBUG("Looking for script module {} [{}]" , name , hash);

    auto* module = GetScriptModule(hash);
    if (module == nullptr) {
      OE_ERROR("Script module {} not found" , name);
    }

    return module;
  }
  
  ScriptModule* CsModule::GetScriptModule(const UUID& id) {
    if (!load_success) {
      OE_WARN("Attempting to get script module {} when C# module is not loaded" , id);
      return nullptr;
    }

    if (loaded_modules.find(id) != loaded_modules.end()) {
      return loaded_modules[id];
    }

    OE_ERROR("Script module w/ ID [{}] not found" , id.Get());
    return nullptr;
  }

  ScriptModule* CsModule::LoadScriptModule(const ScriptModuleInfo& module_info) {    
    std::string case_insensitive_name;
    std::transform(module_info.name.begin() , module_info.name.end() , std::back_inserter(case_insensitive_name) , ::toupper);

    UUID id = FNV(case_insensitive_name);
    OE_DEBUG("Loading C# script module {} [{}]" , module_info.name , id);

    if (loaded_modules.find(id) != loaded_modules.end()) {
      OE_ERROR("  > Script module {} already loaded" , module_info.name);
      return loaded_modules[id];
    } 

    loaded_modules[id] = new CsScript(GetEngine() , root_domain , app_domain , module_info.paths[0]);
    loaded_modules[id]->Initialize();

    return loaded_modules[id];
  }
      
  void CsModule::UnloadScriptModule(const std::string& name) {
    std::string case_insensitive_name;
    std::transform(name.begin() , name.end() , std::back_inserter(case_insensitive_name) , ::toupper);

    UUID id = FNV(case_insensitive_name);
    OE_DEBUG("Unloading C# script module {} [{}]" , name , id);

    if (loaded_modules.find(id) == loaded_modules.end()) {
      OE_ERROR("Attempting to unload script module {} that is not loaded" , name);
      return;
    }

    loaded_modules[id]->Shutdown();
    delete loaded_modules[id];
    loaded_modules.erase(id);
  }

} // namespace other

other::Plugin* CreatePlugin(other::Engine* engine) {
  other::Logger::SetLoggerInstance(engine->GetLog());
  return new other::CsModule(engine);
}

void DestroyPlugin(other::Plugin* module) {
  delete module;
}
