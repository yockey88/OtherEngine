/**
 * \file scripting/lua/lua_module.cpp
 **/
#include "scripting/lua/lua_module.hpp"

#include "core/filesystem.hpp"
#include "application/app_state.hpp"

#include "scripting/lua/lua_error_handlers.hpp"
#include "scripting/lua/lua_bindings.hpp"
#include "scripting/lua/lua_script.hpp"
#include <cctype>
#include <iterator>

namespace other {

  Ref<LuaScript> LuaModule::GetRawScriptHandle(const std::string_view name) {
    UUID id = FNV(name);
    Ref<ScriptModule> module = GetScriptModule(id);
    if (module == nullptr) {
      OE_ERROR("Script module {} not found" , name);
      return nullptr;
    }

    return Ref<ScriptModule>::Cast<LuaScript>(module);
  }

  bool LuaModule::Initialize() {
    OE_DEBUG("Initializing Lua Module");
    OE_DEBUG("Lua Module Initialized");

    load_success = false;
    try {
      context.set_exception_handler(LuaExceptionHandler);
      lua_script_bindings::BindAll(context);

      context.open_libraries(sol::lib::base , sol::lib::package);
      context.require_file("other", "./OtherEngine-ScriptCore/lua/core/other.lua");
      context.require_file("other.behavior", "./OtherEngine-ScriptCore/lua/core/other_behavior.lua");
      context.require_file("other.object", "./OtherEngine-ScriptCore/lua/core/other_object.lua");
      context.require_file("other.scene", "./OtherEngine-ScriptCore/lua/scene/scene.lua");

      load_success = true;
    } catch (const std::exception& e) {
      OE_ERROR("Failed to initialize Lua module : {}" , e.what());
    } catch (...) {
      OE_ERROR("Failed to initialize Lua module : unknown error");
    }

    return load_success;
  }

  void LuaModule::Shutdown() {
    for (auto& [id , module] : loaded_modules) {
      module->Shutdown();
      module = nullptr;
    }
    loaded_modules.clear();

    context.collect_garbage();
    context.stack_clear();
    context.clear_package_loaders();

    OE_DEBUG("Lua Module Shutdown");
  }
  
  void LuaModule::Reload() {
    Shutdown();
    Initialize();

    for (const auto& [id , data] : loaded_modules_data) {
      LoadScriptModule(data);
    }

    load_success = true;
  }

  Ref<ScriptModule> LuaModule::GetScriptModule(const std::string_view name) {
    std::string case_ins_name;
    std::transform(name.begin() , name.end() , std::back_inserter(case_ins_name) , ::toupper);

    UUID id = FNV(case_ins_name);
    Ref<ScriptModule> module = GetScriptModule(id);
    if (module == nullptr) {
      OE_ERROR("Script module {} not found" , name);
    }

    return module;
  }

  Ref<ScriptModule> LuaModule::GetScriptModule(const UUID& id) {
    auto itr = loaded_modules.find(id);
    if (itr != loaded_modules.end()) {
      return itr->second;
    }

    OE_ERROR("Script module {} not found" , id);
    return nullptr;
  }

  Ref<ScriptModule> LuaModule::LoadScriptModule(const ScriptMetadata& module_info) {
    if (!Filesystem::PathExists(module_info.path)) {
      OE_ERROR("Script module {} ({}) does not exist" , module_info.name , module_info.path);
      return nullptr;
    }

    std::string case_insensitive_name;
    std::transform(module_info.name.begin() , module_info.name.end() , std::back_inserter(case_insensitive_name) , ::toupper);
    UUID id = FNV(case_insensitive_name);
    OE_DEBUG("Loading Lua script module {} [{}]" , module_info.name , id);

    if (loaded_modules.find(id) != loaded_modules.end()) {
      OE_ERROR("Attempting to load script module {} that is already loaded" , module_info.name);
      return nullptr;
    }

    auto& m = loaded_modules[id] = NewRef<LuaScript>(context , module_info.path , module_info.name);
    m->Initialize();
    loaded_modules_data[id] = module_info;
    OE_DEBUG("Script module {} ({}) loaded" , module_info.name , module_info.path);

    return m;
  }

  void LuaModule::UnloadScript(const std::string& name) {
    std::string case_insensitive_name;
    std::transform(name.begin() , name.end() , std::back_inserter(case_insensitive_name) , ::toupper);

    UUID id = FNV(case_insensitive_name);
    OE_DEBUG("Unloading C# script module {} [{}]" , name , id);

    if (loaded_modules.find(id) == loaded_modules.end()) {
      OE_ERROR("Attempting to unload script module {} that is not loaded" , name);
      return;
    }

    loaded_modules[id]->Shutdown();
    loaded_modules[id] = nullptr;
    loaded_modules.erase(id);
  }

  std::string_view LuaModule::GetModuleName() const {
    return "Lua";
  }

  std::string_view LuaModule::GetModuleVersion() const {
    return "0.0.1";
  }

} // namespace other
