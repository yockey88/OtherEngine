/**
 * \file scripting/cs/cs_module.cpp
*/
#include "scripting/cs/cs_module.hpp"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/threads.h>

#include "core/filesystem.hpp"
#include "event/event_queue.hpp"
#include "event/app_events.hpp"
#include "scripting/cs/cs_script.hpp"
#include "scripting/cs/script_bindings.hpp"
#include "scripting/cs/script_cache.hpp"

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

    mono_domain_set(app_domain , 0);

    OE_DEBUG("Mono Runtime initialized");

    std::filesystem::path mono_core = std::filesystem::absolute(kEngineCoreDir) / "bin" / "Debug" / "OtherEngine-CsCore" / "OtherEngine-CsCore.dll";
    if (!Filesystem::FileExists(mono_core.string())) {
      OE_ERROR("Mono core assembly {} not found, unloading C# module" , mono_core.string());

      mono_domain_set(root_domain , 0);
      mono_domain_unload(app_domain);
      mono_jit_cleanup(root_domain);
      return load_success;
    }
    
    CsScriptCache::InitializeCache();
    /// CsGarbageCollector::InitializeGC();

    LoadScriptModule(ScriptModuleInfo {
      .name = "C#-Core" ,
      .paths = { 
        mono_core.string()
      }
    });

    load_success = true;
    return load_success;
  }

  void CsModule::Shutdown() {
    if (!load_success) {
      return;
    }
    
    CsScriptCache::ShutdownCache();
    CsScriptBindings::ShutdownBindings();
    
    for (auto& [id , module] : loaded_modules) {
      module->Shutdown();
      delete module;
    }
    loaded_modules.clear();

    /// CsGarbageCollector::ShutdownGC();

    mono_domain_set(root_domain , 0);
    mono_domain_unload(app_domain);
    mono_jit_cleanup(root_domain);

    app_domain = nullptr;
    root_domain = nullptr;

    OE_DEBUG("Mono Runtime shutdown");
  }

  void CsModule::Reload() {
    bool need_to_reload = false;
    for (auto& [id , mod] : loaded_modules) {
      if (mod->HasChanged()) {
        need_to_reload = true;
        break;
      }
    }

    if (!need_to_reload) {
      return;
    }

    for (auto& [id , module] : loaded_modules) {
      module->Shutdown();
      module->Reload(); /// should be called rebuild
      delete module;
    }
    loaded_modules.clear();
    
    /// CsGarbageCollector::ShutdownGC();
    CsScriptCache::ShutdownCache();
    // CsScriptBindings::ShutdownBindings();

    mono_domain_set(root_domain, 0);
    mono_domain_unload(app_domain);

    OE_DEBUG("Reloading Mono Runtime");

    char* app_domain_name = const_cast<char*>(kAppDomainName.data());
    app_domain = mono_domain_create_appdomain(app_domain_name , nullptr);

    mono_domain_set(app_domain , true);

    OE_DEBUG("Mono Runtime initialized");
    
    CsScriptCache::InitializeCache();

    for (const auto& [id , module_info] : loaded_modules_data) {
      LoadScriptModule(module_info);
    }

    EventQueue::PushEvent<ScriptReloadEvent>();
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

    CsScript* script = new CsScript(root_domain , app_domain , module_info.paths[0]);
    loaded_modules[id] = script;
    loaded_modules[id]->Initialize();

    loaded_modules_data[id] = module_info;

    if (id.Get() == FNV("C#-CORE")) {
      CsScriptBindings::InitializeBindings(script->GetImage());
    }

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
