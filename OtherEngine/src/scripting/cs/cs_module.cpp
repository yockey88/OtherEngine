/**
 * \file scripting/cs/cs_module.cpp
*/
#include "scripting/cs/cs_module.hpp"

#include <filesystem>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/threads.h>

#include "core/filesystem.hpp"
#include "core/platform.hpp"
#include "application/app.hpp"
#include "scripting/script_engine.hpp"
#include "scripting/cs/cs_script.hpp"
#include "scripting/cs/cs_script_bindings.hpp"
#include "scripting/cs/cs_garbage_collector.hpp"

namespace other {

  MonoDomain* CsModule::RootDomain() const {
    return root_domain;
  }

  MonoDomain* CsModule::AppDomain() const {
    return app_domain;
  }

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
    
    bool blocking = true;
    CsGarbageCollector::Collect(blocking);
    
    for (auto& [id , module] : loaded_modules) {
      module->Shutdown();
      delete module;
    }
    loaded_modules.clear();
    
    CsScriptBindings::ShutdownBindings();
    CsGarbageCollector::Shutdown();

    mono_domain_set(root_domain , 0);
    mono_domain_unload(app_domain);
    mono_jit_cleanup(root_domain);

    app_domain = nullptr;
    root_domain = nullptr;

    OE_DEBUG("Mono Runtime shutdown");
  }

  void CsModule::Reload() {
    bool blocking = true;
    CsGarbageCollector::Collect(blocking);

    CsScriptBindings::ShutdownBindings();
    CsGarbageCollector::Shutdown();

    mono_domain_set(root_domain, 0);
    mono_domain_unload(app_domain);

    OE_DEBUG("Reloading Mono Runtime");
    
    App* app_ctx = ScriptEngine::GetAppContext();

    auto proj = app_ctx->GetProjectContext();
    auto script_file = proj->GetMetadata().cs_project_file;

    for (auto& [id, module] : loaded_modules) {
      module->Shutdown(); 
      delete module;
    }
    loaded_modules.clear();
    
    OE_DEBUG("Kicking off build for scripts {}" , script_file);

    if (!PlatformLayer::BuildProject(script_file)) {
      OE_ERROR("Failed to rebuild project scripts");
    }
    
    auto editor_file = proj->GetMetadata().cs_editor_project_file;

    OE_DEBUG("Kicking off build for scripts {}" , editor_file);

    if (!PlatformLayer::BuildProject(editor_file)) {
      OE_ERROR("Failed to rebuild editor scripts");
    }

    char* app_domain_name = const_cast<char*>(kAppDomainName.data());
    app_domain = mono_domain_create_appdomain(app_domain_name , nullptr);

    mono_domain_set(app_domain , true);

    OE_DEBUG("Mono Runtime initialized");
    
    for (const auto& [id , module_info] : loaded_modules_data) {
      LoadScriptModule(module_info);
    }
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

    Path module_path = module_info.paths[0];
    std::string mod_path_str = module_path.filename().string();
    std::string mod_name = mod_path_str.substr(0 , mod_path_str.find_last_of('.'));
    CsScript* script = new CsScript(root_domain , app_domain , module_info.paths[0] , mod_name);
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
