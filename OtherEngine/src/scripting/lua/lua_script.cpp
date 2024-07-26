/**
 * \file scripting/lua/lua_script.cpp
 **/
#include "scripting/lua/lua_script.hpp"

#include <algorithm>
#include <iterator>

#include <sol/forward.hpp>
#include <sol/load_result.hpp>

#include "core/filesystem.hpp"
#include "scripting/lua/lua_error_handlers.hpp"
#include "scripting/lua/lua_bindings.hpp"
#include "scripting/lua/lua_math_bindings.hpp"

namespace other {

  void LuaScript::Initialize() {
    bool corrupt = false;
    if (Path{ path }.extension() != ".lua" || !Filesystem::FileExists(path)) {
      OE_ERROR("Failed to load lua script {}" , path);
      corrupt = true;
      return;
    }

    if (corrupt) {
      return;
    }

    try {
      lua_state.set_exception_handler(LuaExceptionHandler);
      sol::function_result res = lua_state.safe_script_file(path);
      if (!res.valid()) {
        OE_ERROR("Failed to load lua script file {}" , path);
        return;
      }

      std::vector<std::string> names;
      const auto& globals = lua_state.globals();
      for (auto& [id , g] : globals) {
        std::string name = id.as<std::string>();

        if (name.find("sol.") != std::string::npos) {
          continue; 
        }

        if (g.get_type() != sol::type::table) {
          continue;
        }

        OE_DEBUG("Lua Object : {}" , name);
        loaded_tables.push_back(name);
      }

      lua_script_bindings::BindGlmTypes(lua_state);
      lua_script_bindings::BindCoreTypes(lua_state);
    } catch (const std::exception& e) {
      OE_ERROR("Failed to load lua script file {}" , path);
      OE_ERROR("  > Error = {}" , e.what());
      return;
    } catch (...) {
      OE_ERROR("Unknown error creating lua state for script {}" , path);
      return;
    }

    valid = true;
  }

  void LuaScript::Shutdown() {
    lua_state.collect_garbage();
    for (auto& [id , obj] : loaded_objects) {
      obj.Shutdown();
    }
    loaded_objects.clear();

    lua_state = sol::state();
  }

  void LuaScript::Reload() {
    /// no need to build anything here
  }
      
  bool LuaScript::HasScript(UUID id) {
    return false; 
  }

  bool LuaScript::HasScript(const std::string_view name , const std::string_view nspace) {
    if (!valid) {
      return false;
    }

    UUID id = FNV(name);
    if (loaded_objects.find(id) != loaded_objects.end()) {
      return true;
    }

    try {
      if (nspace.empty()) {
        auto obj = lua_state[name];
        if (obj.valid()) {
          return true;
        }
      } else {
        auto table = lua_state[nspace];
        if (!table.valid()) {
          return false;
        }

        auto obj = table[name];
        if (obj.valid()) {
          return true;
        }
      }

      return false;
    } catch (std::exception& e) {
      OE_ERROR("Error searching for Lua Script {}::{} : {}" , nspace , name , e.what());
      return false;
    } catch (...) {
      OE_ERROR("Unkown Error searching for Lua Script {}::{}" , nspace , name);
      return false;
    }

    OE_ASSERT(false , "UNREACHABLE CODE");
    return false;
  }

  ScriptObject* LuaScript::GetScript(const std::string& name , const std::string& nspace) {
    if (!valid) {
      return nullptr;
    }
    
    UUID id = FNV(name);
    if (loaded_objects.find(id) != loaded_objects.end()) {
      LuaObject* obj = &loaded_objects[id];
      if (obj != nullptr) {
        return obj;
      }
    }

    OE_INFO("Lua object {} loaded" , name);
  
    auto& obj = loaded_objects[id] = LuaObject(module_name , name , &lua_state);
    obj.InitializeScriptMethods();
    obj.InitializeScriptFields();

    return &loaded_objects[id];
  }
      
  /// TODO: how to get all tables in the script
  std::vector<ScriptObjectTag> LuaScript::GetObjectTags() {
    std::vector<ScriptObjectTag> tags;
    for (auto& table : loaded_tables) {
      std::string case_ins_name;
      std::ranges::transform(table , std::back_inserter(case_ins_name) , ::toupper);

      tags.push_back({
        .object_id = FNV(case_ins_name) ,
        .name = table ,
        .mod_name = module_name ,
        .nspace = "" ,
        .path = path ,
        .lang_type = language ,
      });
    }
    return tags;
  }

} // namespace other
