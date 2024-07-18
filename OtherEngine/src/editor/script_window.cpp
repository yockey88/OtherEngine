/**
 * \file editor/script_window.cpp
 **/
#include "editor/script_window.hpp"

#include <fstream>

#include "core/logger.hpp"

#include "application/app_state.hpp"

#include "scripting/script_defines.hpp"
#include "rendering/ui/ui_helpers.hpp"

#include "editor/script_editor.hpp"

namespace other {

  ScriptWindow::ScriptWindow(bool is_editor_script)
      : UIWindow("Script Creator") , editor_script(is_editor_script) {
    OE_DEBUG("ScriptWindow::ScriptWindow");
    PushRenderFunction(UI_FUNC(ScriptWindow::DrawName));
  }
      
  void ScriptWindow::DrawName() {
    memset(buffer.data() , 0 , buffer.size());
    memcpy(buffer.data() , name.c_str() , name.length());

    if (ImGui::InputText("##script-name" , buffer.data() , buffer.size())) {
      name = buffer.data();
    }
    ui::Underline();

    if (ImGui::Button("confirm")) {        
      PopFrontFunction();
      PushRenderFunction(UI_FUNC(ScriptWindow::DrawOptions));
    }
  }

  void ScriptWindow::DrawOptions() {
    if (!path.has_value()) {
      const char* exts[] = {
        "C#" , "Lua"
      };

      if (ui::Property("Editor Script" , &editor_script)) {
      }

      if (ui::PropertyDropdown("Script Language" , exts , 2 , ext_idx)) {
      }

      if (ImGui::Button("Confirm")) {
        OE_DEBUG("Creating script...");
        switch (ext_idx) {
          case 0: path = name + ".cs"; break;
          case 1: path = name + ".lua"; break;
          default:
            OE_ASSERT(false , "Corrupt script extension choice!");
            return;
        } 
        WriteSource();
        OE_ASSERT(path.has_value() , "Did not assign path value after writing script source!");
        
        auto script_editor = NewRef<ScriptEditor>(name);
        script_editor->AddEditor({
          .object_id = FNV(name) ,
          .name = name ,
          .path = path.value().string() ,
          .lang_type = (ext_idx == 0) ?
            LanguageModuleType::CS_MODULE : LanguageModuleType::LUA_MODULE ,
        }, path.value().string());

        OE_DEBUG("Pushing script editor");
        AppState::PushUIWindow(script_editor);
        
        Close();
      }
    }
  }

  void ScriptWindow::WriteSource() {
    std::string template_src = GetSourceTemplate();
    std::stringstream ss;

    for (auto itr = template_src.begin(); itr != template_src.end();) {
      if (*itr == '@') {
        auto tag_begin = itr;
        while (*itr != ' ') {
          ++itr;
        }
        auto tag_end = itr;

        const std::string tag(tag_begin , tag_end);

        if (tag == "@classname") {
          ss << name;
        } else if (tag == "@baseobject") {
          if (editor_script) {
            ss << "Editor";
          } else {
            ss << "OtherObject";
          }
        }

      } else {
        ss << *itr;
        ++itr;
      }
    }

    source = ss.str();
          
    auto project = AppState::ProjectContext();
    auto dir = (editor_script) ?
      "editor" : "scripts";
    std::string script_path = (project->GetMetadata().assets_dir / dir).string();
    std::string real_path = script_path + '/' + path.value().string();

    std::ofstream script_file(real_path);
    if (!script_file.is_open()) {
      OE_ERROR("Failed to create script file");
      source = std::nullopt;
      /// repush the name functon on and pop this function
      return;
    }

    script_file << source.value();
    script_file.close();

    path = real_path;
  }
      
  std::string ScriptWindow::GetSourceTemplate() const {
    std::string real_template;
    switch (ext_idx) {
      case 0:
        return std::string{ cs_script_template };
      case 1:
        return std::string{ lua_script_template };
      default:
        OE_ASSERT(false , "Corrupt script extension choice in source generation!");
        return "";
    }
  }

} // namespace other
