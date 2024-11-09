/**
 * \file scripting/script_engine.hpp
 */
#include "scripting/script_engine.hpp"

#include <ranges>
#include <string_view>

#include <spdlog/cfg/helpers.h>

#include "core/config_keys.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"

#include "application/app_state.hpp"

#include "ecs/components/script.hpp"

#include "scripting/language_module.hpp"
#include "scripting/script_defines.hpp"
#include "scripting/script_module.hpp"

#include "script_object.hpp"

namespace other {

  ConfigTable ScriptEngine::config;

  Ref<Scene> ScriptEngine::scene_context = nullptr;

  std::vector<ScriptObjectTag> ScriptEngine::object_tags;
  std::map<UUID, LanguageModuleMetadata> ScriptEngine::language_modules;
  std::map<UUID, Ref<ScriptModule>> ScriptEngine::loaded_modules;
  std::map<UUID, Ref<ScriptObject>> ScriptEngine::objects;
  std::map<UUID, Ref<ScriptObject>> ScriptEngine::attachments;

  constexpr static std::array<Ref<LanguageModule> (*)(), kNumModules> kModuleGetters = {
    []() -> Ref<LanguageModule> { return Ref<CsModule>::Create(); },
    []() -> Ref<LanguageModule> { return Ref<LuaModule>::Create(); },
    // []() -> Ref<LanguageModule> { return Ref<PythonModule>::Create(); } ,
  };

  // NativeObjectProxyRegistry native_object_registry = {};

  void ScriptEngine::Initialize(const ConfigTable& cfg) {
    OE_DEBUG("ScriptEngine::Initialize: Initializing ScriptEngine");

    config = cfg;

    LoadModule(CS_MODULE);
    LoadModule(LUA_MODULE);

    // constexpr std::string_view core_lua_name = "OtherEngine.LuaCore";
    // loaded_modules[FNV(core_lua_name)] = lua_language_module->LoadScriptModule({
    //   .name = std::string{ core_lua_name } ,
    //   .path = "./OtherEngine-ScriptCore/lua/core/other.lua"  ,
    // });
  }

  void ScriptEngine::LoadProjectModules() {
    LoadCoreModules();
    // LoadScripts();
  }

  void ScriptEngine::LoadAttachments(const std::string_view section) {
    OE_DEBUG("Loading attachments for section {}", section);

    Ref<LanguageModule> cs_language_module = ScriptEngine::GetModule(CS_MODULE);
    Ref<LanguageModule> lua_language_module = ScriptEngine::GetModule(LUA_MODULE);
    OE_ASSERT(cs_language_module != nullptr, "Failed to retrieve C# language module");
    OE_ASSERT(lua_language_module != nullptr, "Failed to retrieve Lua language module");

    std::string cs_config_section = std::string{ kScriptEngineSection } + "." + std::string{ kCsModuleSection };
    std::string lua_config_section = std::string{ kScriptEngineSection } + "." + std::string{ kLuaModuleSection };
    std::string attachment_key = std::string{ kAttachmentsValue } + "." + std::string{ section };

    auto cs_attachments = config.Get(cs_config_section, attachment_key);
    auto lua_attachments = config.Get(lua_config_section, attachment_key);

    ScriptType script_type = ScriptType::EDITOR_SCRIPT;
    if (section == "scene") {
      script_type = ScriptType::SCENE_SCRIPT;
    }

    if (!cs_attachments.empty()) {
      Ref<Directory> script_bin = Filesystem::GetDirectory("script-bin");
      if (script_bin == nullptr) {
        script_bin = Filesystem::GetDirectory("bin");
      }
      OE_ASSERT(script_bin != nullptr, "Failed to retrieve script bin directory");

      Ref<FileHandle> cs_file = script_bin->OpenFile(cs_attachments[0]);
      if (cs_file == nullptr) {
        OE_ERROR("Failed to load C# attachment file {} ({})", cs_attachments[0], (*script_bin) / Path(cs_attachments[0]));
        return;
      }
      OE_ASSERT(cs_file != nullptr, "Failed to load C# attachment file {}", cs_attachments[0]);

      OE_DEBUG("Loading C# attachment file {}", cs_file->ProjectRelativePath());
      LoadScriptFile(script_type, cs_file);
    }

    if (!lua_attachments.empty()) {
      Ref<FileHandle> lua_file = nullptr;

      Ref<Directory> lua_dir = Filesystem::GetDirectory("lua");
      if (lua_dir == nullptr) {
        lua_dir = script_type == ScriptType::EDITOR_SCRIPT ? Filesystem::GetDirectory("editor") : Filesystem::GetDirectory("scripts");
        OE_ASSERT(lua_dir != nullptr, "Failed to retrieve lua directory");
      }

      lua_file = lua_dir->OpenFile(lua_attachments[0]);
      if (lua_file == nullptr) {
        OE_ERROR("Failed to load Lua attachment file {}", lua_attachments[0]);
        return;
      }
      OE_ASSERT(lua_file != nullptr, "Failed to load Lua attachment file {}", lua_attachments[0]);

      OE_DEBUG("Loading Lua attachment file {}", lua_file->ProjectRelativePath());
      LoadScriptFile(script_type, lua_file);
    }
  }

  void ScriptEngine::AttachObjects() {
    auto attachment_keys = config.GetKeys(kAttachmentsValue);
    for (auto& key : attachment_keys) {
      std::vector<std::string> split_key = key | std::views::split('.') | std::ranges::to<std::vector<std::string>>();
      OE_ASSERT(split_key.size() == 2, "Invalid attachment key : {}", key);

      std::vector<std::string> objs = config.Get(kAttachmentsValue, key, /* case_sensitive = */ true);
      OE_DEBUG("Loading attachments for key {} [{} attachments]", key, objs.size());

      std::string_view module = split_key[1];

      /// FIXME: find a more scalable, consistent way to do this
      // std::string_view category = split_key[0];
      Ref<ScriptModule> mod = GetScriptModule(module);
      if (mod == nullptr) {
        OE_ERROR("Failed to retrieve script module {}", module);
        continue;
      }

      for (auto& obj : objs) {
        Ref<ScriptObject> script_obj = mod->GetScriptObject(obj);
        if (script_obj == nullptr) {
          OE_ERROR("Failed to retrieve script object {} from module {}", obj, module);
          continue;
        }

        script_obj->Initialize();
        attachments[FNV(obj)] = script_obj;
      }
    }
  }

  void ScriptEngine::UpdateAttachments(float dt) {
    for (auto& [id, obj] : attachments) {
      obj->EarlyUpdate(dt);
    }
    for (auto& [id, obj] : attachments) {
      obj->Update(dt);
    }
    for (auto& [id, obj] : attachments) {
      obj->LateUpdate(dt);
    }
  }

  void ScriptEngine::RenderAttachments() {
    for (auto& [id, obj] : attachments) {
      obj->Render();
    }
  }

  void ScriptEngine::RenderUIAttachments() {
    for (auto& [id, obj] : attachments) {
      obj->RenderUI();
    }
  }

  void ScriptEngine::Shutdown() {
    for (auto& [id, mod] : language_modules) {
      mod.module->Shutdown();
      mod.module = nullptr;
    }
  }

  void ScriptEngine::UnloadProjectModules() {
    Ref<LanguageModule> lua_language_module = ScriptEngine::GetModule(LUA_MODULE);
    Ref<LanguageModule> cs_language_module = ScriptEngine::GetModule(CS_MODULE);

    cs_language_module->UnloadScript("OtherEngine.CsCore");
    loaded_modules.clear();
  }

  void ScriptEngine::UnloadAttachments() {
    DetachObjects();
  }

  void ScriptEngine::DetachObjects() {
    for (auto& [id, obj] : attachments) {
      obj->Shutdown();
      obj = nullptr;
    }
    attachments.clear();
  }

  std::string ScriptEngine::GetProjectAssemblyDir() {
    auto project_ctx = AppState::ProjectContext();
    OE_DEBUG("{}/bin/{}", project_ctx->GetFilePath(), "Debug");

    return "";
  }

  void ScriptEngine::ReloadAllScripts() {
    std::vector<UUID> old_loaded_modules;

    for (auto& [id, mod] : loaded_modules) {
      old_loaded_modules.push_back(id);
    }

    objects.clear();
    loaded_modules.clear();

    for (auto& [id, mod] : language_modules) {
      mod.module->Reload();
    }

    object_tags.clear();

    /// @todo: clean this up
    for (auto& id : old_loaded_modules) {
      for (auto& [lid, lang] : language_modules) {
        if (lang.module->HasScript(id)) {
          auto& loaded_module = loaded_modules[id] = lang.module->GetScriptModule(id);
          auto objs = loaded_module->GetObjectTags();

          for (auto& o : objs) {
            object_tags.push_back(o);
          }
        }
      }
    }
  }

  Ref<LanguageModule> ScriptEngine::GetModule(LanguageModuleType type) {
    if (type >= LanguageModuleType::INVALID_LANGUAGE_MODULE) {
      return nullptr;
    }

    return Ref<LanguageModule>::Clone(language_modules[type].module);
  }

  const std::vector<ScriptObjectTag>& ScriptEngine::GetLoadedObjects() {
    return object_tags;
  }

  Ref<ScriptModule> ScriptEngine::LoadScriptModule(const Path& path) {
    return nullptr;
  }

  Ref<ScriptModule> ScriptEngine::GetScriptModule(const std::string_view name) {
    Ref<ScriptModule> mod = GetScriptModule(FNV(name));
    if (mod != nullptr) {
      return mod;
    }

    OE_DEBUG("Searching for script module {}", name);
    for (auto& [lid, lang] : language_modules) {
      OE_DEBUG(" > Searching in loaded language module {}", lang.module->GetModuleName());
      if (!lang.module->HasScript(name)) {
        continue;
      }

      mod = lang.module->GetScriptModule(std::string{ name });
      loaded_modules[FNV(mod->ModuleName())] = mod;
      OE_DEBUG("  > Found script module {} in language module {}", mod->ModuleName(), lang.module->GetModuleName());
      return mod;
    }

    OE_ERROR("Failed to retrieve script module {}", name);
    return nullptr;
  }

  Ref<ScriptModule> ScriptEngine::GetScriptModule(UUID id) {
    auto itr = std::find_if(loaded_modules.begin(), loaded_modules.end(), [&](const auto& module) -> bool {
      return module.first == id;
    });
    if (itr != loaded_modules.end()) {
      return itr->second;
    }

    return nullptr;
  }

  Ref<ScriptObject> ScriptEngine::GetScriptObject(UUID id) {
    auto itr = objects.find(id);
    if (itr != objects.end()) {
      return itr->second.Raw();
    }

    return nullptr;
  }

  Ref<ScriptObject> ScriptEngine::GetScriptObject(const std::string_view name) {
    if (name.empty()) {
      OE_ERROR("ScriptEngine::GetScriptObject([NULL NAME])");
      return nullptr;
    }

    return GetScriptObject(name, "");
  }

  Ref<ScriptObject> ScriptEngine::GetScriptObject(const std::string_view name, const std::string_view nspace) {
    std::string search_name = std::string{ name };
    std::string search_nspace = std::string{ nspace };

    if (nspace.empty()) {
      const auto [sname, nspace] = ParseScriptName(name);
      search_name = sname;
      search_nspace = nspace;
    }

    for (auto& [id, mod] : loaded_modules) {
      if (mod->HasScript(search_name, search_nspace)) {
        Ref<ScriptObject> obj = mod->GetScriptObject(search_name, search_nspace);
        if (obj == nullptr) {
          OE_ERROR("Failed to retrieve script object {}::{} from module {} (module corrupted)", search_nspace, search_name, mod->ModuleName());
          return nullptr;
        }

        UUID id = FNV(name);
        objects[id] = obj;
        return obj;
      }
    }

    for (auto& [id, lang] : language_modules) {
      for (auto& [lid, mod] : lang.module->GetModules()) {
        OE_TRACE("Searching for script object {}::{} in module {}", search_nspace, search_name, mod->ModuleName());
        if (mod->HasScript(search_name, search_nspace)) {
          loaded_modules[FNV(mod->ModuleName())] = mod;

          Ref<ScriptObject> obj = mod->GetScriptObject(search_name, search_nspace);
          if (obj == nullptr) {
            OE_ERROR("Failed to retrieve script object {}::{} from module {} (module corrupted)", search_nspace, search_name, mod->ModuleName());
            return nullptr;
          }

          UUID id = FNV(name);
          objects[id] = obj;
          return obj;
        }
      }
    }

    OE_ERROR("ScriptEngine::GetScriptObject({}.{}) -> failed to find script, no appropriate module found", search_nspace, search_name);
    return nullptr;
  }

  Ref<ScriptObject> ScriptEngine::GetScriptObject(const std::string_view name, const std::string_view nspace, const std::string_view mod_name) {
    if (mod_name.empty()) {
      OE_ERROR("ScriptEngine::GetScriptObject({}.{} [NULL MODULE])", nspace, name);
      return nullptr;
    }

    std::string search_name = std::string{ name };
    std::string search_nspace = std::string{ nspace };

    if (nspace.empty()) {
      const auto [sname, nspace] = ParseScriptName(name);
      search_name = sname;
      search_nspace = nspace;
    }

    Ref<ScriptModule> mod = GetScriptModule(mod_name);
    /// if its null we have the equivalent of the above case so we don't need to return cause we might still find the object
    if (mod == nullptr) {
      return nullptr;
    }

    return mod->GetScriptObject(search_name, search_nspace);
  }

  const std::map<UUID, Ref<ScriptObject>>& ScriptEngine::ReadLoadedObjects() {
    return objects;
  }

  void ScriptEngine::SetSceneContext(const Ref<Scene>& scene) {
    scene_context = scene;
  }

  Ref<Scene> ScriptEngine::GetSceneContext() {
    return scene_context;
  }

  std::map<UUID, LanguageModuleMetadata>& ScriptEngine::GetModules() {
    return language_modules;
  }

  Script ScriptEngine::LoadScriptsFromTable(const ConfigTable& table, const std::string_view section) {
    auto script_paths = table.Get(section, kScriptsValue);

    std::string script_key = std::string{ section } + "." + std::string{ kScriptValue };
    auto script_objs = table.GetKeys(script_key);

    auto project_path = AppState::ProjectContext()->GetMetadata().file_path.parent_path();
    auto assets_dir = AppState::ProjectContext()->GetMetadata().assets_dir;

    OE_DEBUG("Loading Editor Scripts");
    for (auto& mod : script_paths) {
      Path module_path = Path{ mod };
      LoadScriptModule(module_path);
    }

    return CollectObjects(table, script_objs, section);
  }

  LanguageModuleType ScriptEngine::StringToModuleType(const std::string_view name) {
    UUID id = FNV(name);
    return IdToModuleType(id);
  }

  LanguageModuleType ScriptEngine::IdToModuleType(UUID id) {
    switch (id.Get()) {
      case kModuleInfo[CS_MODULE].hash:
        return LanguageModuleType::CS_MODULE;
      case kModuleInfo[LUA_MODULE].hash:
        return LanguageModuleType::LUA_MODULE;
        return LanguageModuleType::INVALID_LANGUAGE_MODULE;
    }

    return LanguageModuleType::INVALID_LANGUAGE_MODULE;
  }

  void ScriptEngine::LoadCoreModules() {
    auto project_metadata = AppState::ProjectContext();
    if (project_metadata == nullptr) {
      OE_ERROR("Failed to load project metadata");
      return;
    } else {
      OE_DEBUG("loading scripts modules from project metadata");
    }

    const Path engine_core_dir = Filesystem::GetEngineCoreDir();
    const Path engine_bin = engine_core_dir / "bin" / "Debug";
    const Path cs_core = engine_bin / "OtherEngine-CsCore" / "net8.0" / "OtherEngine-CsCore.dll";

    Ref<FileHandle> cs_core_dll = Filesystem::RegisterFile(cs_core);

    Ref<LanguageModule> cs_language_module = ScriptEngine::GetModule(CS_MODULE);
    Ref<LanguageModule> lua_language_module = ScriptEngine::GetModule(LUA_MODULE);

    /// load c# core module
    constexpr std::string_view core_cs_name = "OtherEngine.CsCore";
    loaded_modules[FNV(core_cs_name)] = cs_language_module->LoadScriptModule({
      .name = std::string{ core_cs_name },
      .handle = cs_core_dll,
    });
  }

  void ScriptEngine::LoadScripts() {
    Ref<Directory> editor_dir = Filesystem::GetDirectory("editor");
    OE_ASSERT(editor_dir != nullptr, "Failed to load editor directory");

    Ref<Directory> script_dir = Filesystem::GetDirectory("scripts");
    OE_ASSERT(script_dir != nullptr, "Failed to load scripts directory");

    std::vector<Path> editor_scripts = editor_dir->GetFiles();
    std::vector<Path> script_scripts = script_dir->GetFiles();

    for (const auto& script : editor_scripts) {
      Ref<FileHandle> file = Filesystem::GetFile(script);
      OE_ASSERT(file != nullptr, "Failed to load script file, file handle was null : {}", script.string());
      OE_ASSERT(file->Exists(), "Failed to load script file, file does not exist : {}", script.string());

      if (file->GetAssetType() == AssetType::SCRIPTFILE) {
        LoadScriptFile(ScriptType::EDITOR_SCRIPT, file);
      }
    }

    for (const auto& script : script_scripts) {
      Ref<FileHandle> file = Filesystem::GetFile(script);
      OE_ASSERT(file != nullptr, "Failed to load script file, file handle was null : {}", script.string());
      OE_ASSERT(file->Exists(), "Failed to load script file, file does not exist : {}", script.string());

      if (file->GetAssetType() == AssetType::SCRIPTFILE) {
        LoadScriptFile(ScriptType::SCENE_SCRIPT, file);
      }
    }
  }

  void ScriptEngine::LoadScriptFile(ScriptType type, const Ref<FileHandle>& path) {
    OE_ASSERT(path != nullptr, "Failed to load script file, file handle was null");
    OE_ASSERT(path->Exists(), "Failed to load script file, path does not exist : {}", path->AbsolutePath());
    OE_ASSERT(path->GetAssetType() == AssetType::SCRIPTFILE, "Failed to load script file, file is not a script file : {}", path->AbsolutePath());

    if (AppState::mode == EngineMode::RUNTIME && type == ScriptType::EDITOR_SCRIPT) {
      return;
    }

    LanguageModuleType lang_type = ModuleTypeFromExtension(path->Extension());
    if (lang_type == LanguageModuleType::INVALID_LANGUAGE_MODULE) {
      OE_ERROR("Failed to load script file, invalid language module type : {}", path->AbsolutePath());
      return;
    }

    Ref<LanguageModule> module = GetModule(lang_type);
    OE_ASSERT(module != nullptr, "Failed to load script file, language module was null");

    std::string name = path->FileName();
    std::string case_ins_name = "";
    std::transform(name.begin(), name.end(), std::back_inserter(case_ins_name), ::toupper);

    OE_DEBUG("Loading script module {} into [{}] ({})", name, module->GetLanguageType(), path->AbsolutePath());
    Ref<ScriptModule> script = module->LoadScriptModule({
      .name = name,
      .case_ins_name = case_ins_name,
      .handle = path,
    });
    OE_ASSERT(script != nullptr, "Failed to load script module : {}", name);
  }

  void ScriptEngine::LoadModule(LanguageModuleType type) {
    language_modules[type].id = kModuleInfo[type].hash;
    language_modules[type].name = kModuleInfo[type].name;

    language_modules[type].module = kModuleGetters[type]();
    language_modules[type].module->Initialize();
  }

  LanguageModuleType ScriptEngine::ModuleTypeFromExtension(const std::string_view ext) {
    if (ext == ".dll") {
      return LanguageModuleType::CS_MODULE;
    } else if (ext == ".lua") {
      return LanguageModuleType::LUA_MODULE;
    }
    return LanguageModuleType::INVALID_LANGUAGE_MODULE;
  }

  void ScriptEngine::LoadScriptModule(Path& module_path) {
    Ref<FileHandle> file = Filesystem::RegisterFile(module_path);
    if (file == nullptr) {
      OE_ERROR("Failed to load script module : {}", module_path.string());
      return;
    }
    OE_DEBUG("Loading Editor Script Module : {} ({})", file->FileName(), module_path.string());

    LanguageModuleType lang_type = ModuleTypeFromExtension(file->Extension());
    language_modules[lang_type].module->LoadScriptModule({
      .name = file->FileName(),
      .handle = file,
      .type = ScriptType::EDITOR_SCRIPT,
    });
  }

  Script ScriptEngine::CollectObjects(const ConfigTable& table, const std::vector<std::string>& objects, const std::string_view section) {
    Script res{};
    // for (const auto& obj : objects) {
    //   auto scripts = table.Get(section , obj);

    //   Ref<ScriptModule> mod = GetScriptModule(obj);
    //   OE_ASSERT(mod != nullptr , "Failed to get script module {}" , obj);

    //   for (auto& s : scripts) {
    //     OE_DEBUG("Attaching editor script");

    //     std::string nspace = "";
    //     std::string name = s;
    //     if (s.find("::") != std::string::npos) {
    //       nspace = s.substr(0 , s.find_first_of(":"));
    //       OE_DEBUG("Editor script from namespace {}" , nspace);

    //       name = s.substr(s.find_last_of(":") + 1 , s.length() - nspace.length() - 2);
    //       OE_DEBUG(" > with name {}" , name);
    //     }

    //     Ref<ScriptObject> inst = mod->GetScriptObject(name , nspace);
    //     OE_ASSERT(inst != nullptr , "Failed to get script {} from script module {}" , s , obj);
    //     std::string case_ins_name;
    //     std::transform(s.begin() , s.end() , std::back_inserter(case_ins_name) , ::toupper);

    //     UUID id = FNV(case_ins_name);
    //     res.data[id] = ScriptObjectData{
    //       .module = obj ,
    //       .obj_name = s ,
    //     };
    //     Ref<ScriptObject>& obj = res.scripts[id] = inst;
    //     obj->OnBehaviorLoad();
    //   }
    // }

    return res;
  }

  std::pair<std::string, std::string> ScriptEngine::ParseScriptName(const std::string_view full_name) {
    std::string name = std::string{ full_name };
    std::optional<std::string> name_space = std::nullopt;

    if (full_name.find("::") != std::string::npos) {
      auto colon = full_name.find_first_of(':');
      auto second_colon = full_name.find_last_of(':');

      name_space = full_name.substr(0, colon);
      name = full_name.substr(second_colon + 1, full_name.length() - name_space->length() + 2);
    } else if (full_name.find(".") != std::string::npos) {
      auto dot = full_name.find_first_of('.');
      name_space = full_name.substr(0, dot);
      name = full_name.substr(dot + 1, full_name.length() - name_space->length() + 2);
    }

    return { name, name_space.value_or("") };
  }

}  // namespace other
