/**
 * \file editor/panel_manager.cpp
 **/
#include "editor/panel_manager.hpp"

#include "core/config_keys.hpp"

#include "scripting/script_engine.hpp"

#include "editor/project_panel.hpp"
#include "editor/scene_panel.hpp"
#include "editor/entity_properties.hpp"
#include "editor/selection_manager.hpp"

namespace other {

  constexpr UUID kProjectPanelId = FNV("ProjectPanel");
  constexpr UUID kScenePanelId = FNV("ScenePanel");
  constexpr UUID kPropertiesPanelId = FNV("PropertiesPanel");

  void PanelManager::Load(Editor* editor , const Ref<Project>& context) {
    OE_ASSERT(editor != nullptr , "Loading editor panel manager with a null editor!");
    OE_ASSERT(context != nullptr , "Loading editor panel manager with null project context!");

    project_context = context;

    auto& proj_panel = active_panels[kProjectPanelId] = Panel{};
    auto& scene_panel = active_panels[kScenePanelId] = Panel{};
    auto& props_panel = active_panels[kPropertiesPanelId] = Panel{};

    proj_panel.panel_open = true;
    proj_panel.panel = Ref<ProjectPanel>::Create(*editor); 
    proj_panel.panel->OnProjectChange(project_context);

    scene_panel.panel_open = true;
    scene_panel.panel = Ref<ScenePanel>::Create(*editor); 

    props_panel.panel_open = true;
    props_panel.panel = Ref<EntityProperties>::Create(*editor); 
  }

  void PanelManager::Attach(const ConfigTable& editor_config) {
    LoadEditorScripts(editor_config);

    for (const auto& [id , script] : editor_scripts.scripts) {
      script->OnBehaviorLoad();
      script->Initialize();
      script->Start();
    }

    for (auto& [id , panel] : active_panels) {
      panel.panel->OnAttach();
    }
  }

  void PanelManager::OnEvent(Event* event) {
  }
  
  void PanelManager::Update(float dt) {
    active_panels[kPropertiesPanelId].panel_open = SelectionManager::HasSelection();
    
    for (const auto& [id , script] : editor_scripts.scripts) {
      script->Update(dt);
    }
  }
  
  void PanelManager::Render() {
    for (const auto& [id , script] : editor_scripts.scripts) {
      script->Render();
    }
  }

  void PanelManager::RenderUI() {
    for (auto& [id , panel] : active_panels) {
      panel.panel->OnGuiRender(panel.panel_open);
      if (id == kPropertiesPanelId && SelectionManager::HasSelection() && !panel.panel_open) {
        SelectionManager::ClearSelection();
      }
    }
    
    for (const auto& [id , script] : editor_scripts.scripts) {
      script->RenderUI();
    }
  }

  void PanelManager::Detach() {
    for (const auto& [id , script] : editor_scripts.scripts) {
      script->Stop();
      script->Shutdown();
      script->OnBehaviorUnload();
    }
    editor_scripts.scripts.clear();
  }

  void PanelManager::Unload() {
    for (auto& [id , panel] : active_panels) {
      panel.panel_open = false;
      panel.panel = nullptr;
    }
  }

  void PanelManager::OnSceneLoad(const SceneMetadata* scene_metadata) {
    OE_ASSERT(scene_metadata != nullptr , "Attempting to set scene context to null scene in panel manager!");

    for (auto& [id , panel] : active_panels) {
      panel.panel->SetSceneContext(scene_metadata->scene);
    }
  }

  void PanelManager::OnSceneUnload() {
    for (auto& [id , panel] : active_panels) {
      panel.panel->SetSceneContext(nullptr);
    }
  }

  void PanelManager::OnScriptReload() {
    editor_scripts.scripts.clear();
    for (const auto& [id , info] : editor_scripts.data) {
      ScriptModule* mod = ScriptEngine::GetScriptModule(info.module);
      if (mod == nullptr) {
        OE_ERROR("Failed to find editor scripting module {} [{}]" , info.module , FNV(info.module));
        continue;
      }

      std::string nspace = "";
      std::string name = info.obj_name;
      if (name.find("::") != std::string::npos) {
        nspace = name.substr(0 , name.find_first_of(":"));
        OE_DEBUG("Editor script from namespace {}" , nspace);

        name = name.substr(name.find_last_of(":") + 1 , name.length() - nspace.length() - 2);
        OE_DEBUG(" > with name {}" , name);
      }

      ScriptObject* inst = mod->GetScript(name , nspace);
      if (inst == nullptr) {
        OE_ERROR("Failed to get script {} from script module {}" , name , info.module);
        continue;
      } else {
        std::string case_ins_name;
        std::transform(name.begin() , name.end() , std::back_inserter(case_ins_name) , ::toupper);

        UUID id = FNV(case_ins_name);
        auto& obj = editor_scripts.scripts[id] = inst;
        obj->Start();
      }
    }
  }

  void PanelManager::LoadEditorScripts(const ConfigTable& editor_config) {
    Ref<LanguageModule> cs_lang_mod = ScriptEngine::GetModule(LanguageModuleType::CS_MODULE);
    if (cs_lang_mod == nullptr) {
      OE_ERROR("Failed to load editor scripts, C# module is null!");
      return;
    }

    Ref<LanguageModule> lua_module = ScriptEngine::GetModule(LanguageModuleType::LUA_MODULE);
    if (lua_module == nullptr) {
      OE_ERROR("Failed to load editor scripts Lua Module is null!");
      return;
    }

    auto script_paths = editor_config.Get(kEditorSection , kScriptsValue);

    std::string script_key = std::string{ kEditorSection } + "." + std::string{ kScriptValue };
    auto script_objs = editor_config.GetKeys(script_key);

    OE_DEBUG("Loading Editor Scripts");
    for (auto& mod : script_paths) {
      Path module_path = Path{ mod }; 

      std::string fname = module_path.filename().string();
      std::string mname = fname.substr(0 , fname.find_last_of('.'));
      OE_DEBUG("Loading Editor Script Module : {} ({})" , mname , module_path.string());

      if (module_path.extension() == ".dll") {
        cs_lang_mod->LoadScriptModule({
          .name = mname ,
          .paths = { module_path.string() }
        });
      } else if (module_path.extension() == ".lua") {
        lua_module->LoadScriptModule({
          .name = mname ,
          .paths = { module_path.string() }
        });
      }
    }

    for (const auto& obj : script_objs) {
      auto scripts = editor_config.Get(script_key , obj);

      ScriptModule* mod = ScriptEngine::GetScriptModule(obj);
      if (mod == nullptr) {
        OE_ERROR("Failed to find editor scripting module {} [{}]" , obj , FNV(obj));
        continue;
      }

      for (auto& s : scripts) {
        OE_DEBUG("Attaching editor script");

        std::string nspace = "";
        std::string name = s;
        if (s.find("::") != std::string::npos) {
          nspace = s.substr(0 , s.find_first_of(":"));
          OE_DEBUG("Editor script from namespace {}" , nspace);

          name = s.substr(s.find_last_of(":") + 1 , s.length() - nspace.length() - 2);
          OE_DEBUG(" > with name {}" , name);
        }

        ScriptObject* inst = mod->GetScript(name , nspace);
        if (inst == nullptr) {
          OE_ERROR("Failed to get script {} from script module {}" , s , obj);
          continue;
        } else {
          std::string case_ins_name;
          std::transform(s.begin() , s.end() , std::back_inserter(case_ins_name) , ::toupper);

          UUID id = FNV(case_ins_name);
          editor_scripts.data[id] = ScriptObjectData{
            .module = obj ,
            .obj_name = s ,
          };
          auto& obj = editor_scripts.scripts[id] = inst;
          obj->Start();
        }
      }
    }
  }

} // namespace other
