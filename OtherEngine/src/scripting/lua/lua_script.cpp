/**
 * \file scripting/lua/lua_script.cpp
 **/
#include "scripting/lua/lua_script.hpp"

#include <algorithm>
#include <iterator>

#include <sol/error.hpp>
#include <sol/forward.hpp>
#include <sol/load_result.hpp>
#include <sol/types.hpp>

#include "core/filesystem.hpp"
#include "core/ref.hpp"

namespace other {

  void LuaScript::Initialize() {
    bool corrupt = false;

    try {
      sol::function_result res = lua_state.safe_script_file(path);
      if (!res.valid()) {
        OE_ERROR("Failed to load lua script file {}", path);
        return;
      }
    } catch (const std::exception& e) {
      OE_ERROR("Failed to load lua script file {}", path);
      OE_ERROR("  > Error = {}", e.what());
      corrupt = true;
    } catch (...) {
      OE_ERROR("Unknown error creating lua script {}", path);
      corrupt = true;
    }

    if (corrupt) {
      OE_ERROR("Failed to load lua script module {}", module_name);
      return;
    }

    valid = !corrupt;
  }

  void LuaScript::Shutdown() {
    lua_state.collect_garbage();
    for (auto& [id, obj] : loaded_objects) {
      obj->OnBehaviorUnload();
      obj = nullptr;
    }
    loaded_objects.clear();

    for (auto& [id, obj] : objects) {
      obj = nullptr;
    }
    objects.clear();
  }

  void LuaScript::Reload() {
  }

  bool LuaScript::HasScript(UUID id) const {
    return false;
  }

  bool LuaScript::HasScript(const std::string_view name, const std::string_view nspace) const {
    if (!valid) {
      OE_ERROR("Lua script {} is not valid : cannot search for {}.{}", path, name, nspace);
      return false;
    }

    for (const auto& [id, obj] : loaded_objects) {
      if (obj->Name() == name && obj->NameSpace() == nspace) {
        return true;
      }
    }

    if (!nspace.empty()) {
      sol::table table = lua_state.get_or(nspace, sol::nil);
      if (table == sol::nil) {
        OE_ERROR("Failed to find lua table {}", nspace);
        return false;
      }

      if (lua_state.get_or(name, sol::nil) != sol::nil) {
        return true;
      }
    } else {
      if (lua_state.get_or(name, sol::nil) != sol::nil) {
        return true;
      }
    }

    return false;
  }

  Ref<ScriptObject> LuaScript::GetScriptObject(const std::string& name, const std::string& nspace) {
    if (!valid) {
      return nullptr;
    }

    UUID id = FNV(name);
    if (loaded_objects.find(id) != loaded_objects.end()) {
      ScriptRef<LuaObject> obj = loaded_objects[id];
      if (obj != nullptr) {
        return obj;
      }
    }

    ScriptRef<LuaObject> obj = nullptr;
    {
      try {
        sol::table search_table = lua_state.globals();
        if (!nspace.empty()) {
          search_table = lua_state.get_or(nspace, sol::nil);
        }
        OE_ASSERT(search_table != sol::nil, "Failed to find lua table for searching (why is globals() nil?) {}", nspace);

        sol::table object = search_table[name];
        if (object == sol::nil) {
          OE_ERROR("Failed to find lua object {}", name);
          return nullptr;
        }

        std::string real_name = "";
        if (nspace.empty()) {
          real_name = name;
        } else {
          real_name = fmtstr("{}.{}", nspace, name);
        }

        obj = NewRef<LuaObject>(this, id, real_name, lua_state, std::forward<sol::table>(object));
      } catch (const std::exception& e) {
        OE_ERROR("Failed to create Lua object {}", name);
        OE_ERROR("  > Error = {}", e.what());
        obj = nullptr;
      } catch (...) {
        OE_ERROR("Unknown error creating Lua object {}", name);
        obj = nullptr;
      }
    }

    if (obj == nullptr) {
      OE_ERROR("Failed to create Lua object {}", name);
      return nullptr;
    }

    objects[id] = obj;
    loaded_objects[id] = obj;
    obj->InitializeScriptMethods();
    obj->InitializeScriptFields();
    obj->OnBehaviorLoad();

    OE_INFO("Lua object {} loaded", name);
    return obj;
  }

  /// TODO: how to get all tables in the script
  std::vector<ScriptObjectTag> LuaScript::GetObjectTags() {
    std::vector<ScriptObjectTag> tags;
    for (auto& table : loaded_tables) {
      std::string case_ins_name;
      std::ranges::transform(table, std::back_inserter(case_ins_name), ::toupper);

      tags.push_back({
        .object_id = FNV(case_ins_name),
        .name = table,
        .mod_name = module_name,
        .nspace = "",
        .lang_type = language,
      });
    }
    return tags;
  }

}  // namespace other
