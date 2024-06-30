/**
 * \file editor/script_editor.cpp
 **/
#include "editor/script_editor.hpp"

#include <imgui/imgui.h>
#include <zep/syntax.h>

#include "core/defines.hpp"
#include "core/logger.hpp"
#include "core/filesystem.hpp"
#include "rendering/ui/ui_helpers.hpp"
#include "rendering/ui/text_editor.hpp"
#include "scripting/script_defines.hpp"

namespace other {
  
  static std::unordered_set<std::string> cs_keywords = {
    "abstract" , "as" , "base" , "bool" , "break" , "byte" , "case" , "catch" , "char" , "checked" , "class" , "const" , "continue" , 
    "decimal" , "default" , "delegate" , "do" , "double" , "else" , "enum" , "event" , "explicit" , "extern" , 
    "false" , "finally" , "fixed" , "float" , "for" , "foreach" , 
    "goto" , 
    "if" , "implicit" , "in" , "int" , "interface" , "internal" , "is" , 
    "lock" , "long" , "namespace" , "new" , "null" , "object" , "operator" , "out" , "override" , 
    "params" , "private" , "protected" , "public" , "readonly" , "ref" , "return" , "sbyte" , "sealed" , "short" , "sizeof" , 
    "stackalloc" , "static" , "string" , "struct" , "switch" , "this" , "throw" , "true" , "try" , "typeof" , "uint" , "ulong" , 
    "unchecked" , "unsafe" , "ushort" , "using" , "virtual" , "void" , "volatile" , "while" ,

    "add" , "and" , "alias" , "ascending" , "async" , "await" , "by" , "descending" , "dynamic" , "equals" , "from" , "get" , "global" ,
    "group" , "init" , "into" , "join" , "let" , "managed" , "nameof" , "nint" , "not" , "notnull" , "nuint" , "on" , "or" , "orderby" , 
    "partial" , "remove" , "required" , "scoped" , "select" , "set" , "unmanaged" , "value" , "var" , "when" , "where" , "with" , "yield"
  };

  static std::unordered_set<std::string> cs_identifiers = {
    "System" , "IEquatable" , "List" , "Dictionary" ,
    "Attribute" ,
    "Other" , "OtherBehavior" , "OtherObject" ,
    "Logger" , "Renderer" , "Input" , "Scene" ,
    "Component" , "Tag" , "Entity" , "OtherAttribute" , 
    "BoundsAttribute" ,
    "Parent" , "Children" , "Transform" , 
    "Camera" , "Mesh" , "Collider2D" , 
    "Relationship" , "RigidBody2D" , 
    "Script" ,
    "Editor" , "Keyboard" , "Mouse" , 
    "Vec2" , "Vec3" , "Vec4" , "Mat4" , 
    "Quaternion" , 
    "Line" , "Triangle" , "Rect" , "Cube" ,
    "Circle" , "Sphere" , "Capsule" , "Cylinder" , 
    "Color" , "Renderer" , "UI" , "Vertex" , 
  };
  
  static StdRef<Zep::ZepSyntax> GetCSBufferSyntax(Zep::ZepBuffer* buffer) {
    return NewStdRef<Zep::ZepSyntax>(*buffer , cs_keywords , cs_identifiers);
  }

  ScriptEditor::ScriptEditor(Opt<std::string> window_name)
          : UIWindow("Script Editor" + (window_name.has_value() ?
                        (" : " + window_name.value()) : "")) {
    
    PushRenderFunction([this]() {
      ImGui::BeginTabBar("Script-Editor-Tab-Bar" , ImGuiTabBarFlags_DockNode);
      for (auto& editor : active_editors) {
        ImGui::PushID(editor.first.Get());
        Path file_path = editor.second.text_buffer->GetFilePath();

        ImGui::BeginTabItem(file_path.filename().string().c_str() , &editor.second.tab_open);

        const bool has_focus = ImGui::IsWindowFocused();
        auto min = ImGui::GetCursorScreenPos();
        auto max = ImGui::GetContentRegionAvail();

        if (max.x <= 0) { 
          max.x = 1; 
        } 

        if (max.y <= 0) { 
          max.y = 1; 
        } 

        max.x = min.x + max.x;
        max.y = min.y + max.y;

        ImGui::InvisibleButton("EditorContainer" , max);

        for (const auto& [id , buffer] : active_editors) {
          buffer.zep->GetEditor().SetDisplayRegion(Zep::NVec2f(min.x , min.y) , Zep::NVec2f(max.x , max.y));
          buffer.zep->GetEditor().Display();
          buffer.zep->HandleInput();
        }


        if (has_focus) {
          uint32_t focus = 0;
          if (focus++ < 2) {
            ImGui::SetWindowFocus();
          }
        }

        ImGui::EndTabItem();

        ImGui::PopID();
      } 

      ImGui::EndTabBar();
    });
  }

  void ScriptEditor::AddEditor(const ScriptObjectTag& obj_tag , const std::string& script_path) {
    auto engine_core = Filesystem::GetEngineCoreDir();
    auto zep_config = engine_core / ".zep";

    OE_DEBUG("script editor config path : {}" , zep_config.string());

    auto& new_editor = active_editors[obj_tag.object_id] = TextBuffer{}; 
    new_editor.id = obj_tag.object_id;
    new_editor.editor = this;

    new_editor.callback = BIND_FUNC(ScriptEditor::TextBuffer::HandleMessage , &new_editor);
    new_editor.zep = NewScope<ScriptZepWrapper>(zep_config.string() , pixel_scale , new_editor.callback);
    auto& zep_display = new_editor.zep->GetEditor().GetDisplay();
    auto font = ImGui::GetIO().Fonts->Fonts[0];
    auto pixl_h = font->FontSize;
    
    zep_display.SetFont(Zep::ZepTextType::UI , NewStdRef<Zep::ZepFont_ImGui>(zep_display , font , (uint32_t)pixl_h));
    zep_display.SetFont(Zep::ZepTextType::Text , NewStdRef<Zep::ZepFont_ImGui>(zep_display , font , (uint32_t)pixl_h));
    zep_display.SetFont(Zep::ZepTextType::Heading1 , NewStdRef<Zep::ZepFont_ImGui>(zep_display , font , (uint32_t)pixl_h * 1.5));
    zep_display.SetFont(Zep::ZepTextType::Heading2 , NewStdRef<Zep::ZepFont_ImGui>(zep_display , font , (uint32_t)pixl_h * 1.25));
    zep_display.SetFont(Zep::ZepTextType::Heading3 , NewStdRef<Zep::ZepFont_ImGui>(zep_display , font , (uint32_t)pixl_h * 1.125));

    new_editor.zep->GetEditor().GetTheme().SetThemeType(Zep::ThemeType::Dark);
    new_editor.text_buffer = new_editor.zep->GetEditor().InitWithFile(script_path);

    if (obj_tag.lang_type == LanguageModuleType::CS_MODULE) {
      Zep::SyntaxProvider cs_syntax{
        .syntaxID = ".cs" ,
        .factory = GetCSBufferSyntax ,
      };

      std::vector<std::string> mappings = {
        ".cs"
      };

      new_editor.zep->GetEditor().RegisterSyntaxFactory(mappings , cs_syntax);
      new_editor.zep->GetEditor().SetBufferSyntax(*new_editor.text_buffer);
    }
    
    new_editor.tab_open = true;
  }
      
  void ScriptEditor::OnUpdate(float dt) {
    for (const auto& [id , buffer] : active_editors) {
      buffer.zep->GetEditor().RefreshRequired();
    }
  }
      
  void ScriptEditor::Render() {
    UIWindow::Render();
    PostRender();
  }
      
  void ScriptEditor::PostRender() {
    for (auto& id : ids_to_clear) {
      auto itr = active_editors.find(id);
      if (itr == active_editors.end()) {
        continue;
      }

      active_editors.erase(itr);
    }

    if (active_editors.size() == 0) {
      Close();
    }
  }

  void ScriptEditor::TextBuffer::HandleMessage(Zep::MessagePtr message) {
    if (message->handled) {
      return;
    }

    if (message->messageId == Zep::Msg::RequestQuit || message->str == ":wq") {
      zep->GetEditor().SaveBuffer(*text_buffer);
      editor->ids_to_clear.push_back(id);
      message->handled = true;
    } else if (message->str == ":Ex") {
      zep->GetEditor().RequestQuit();
      message->handled = true;
    } else if (message->messageId == Zep::Msg::ToolTip) {
      auto tip_msg = std::static_pointer_cast<Zep::ToolTipMessage>(message); 
      if (!tip_msg->location.Valid() || tip_msg->pBuffer == nullptr) {
        return;
      }
      auto syntax = tip_msg->pBuffer->GetSyntax();
      if (syntax == nullptr) {
        return;
      }

      auto marker = NewStdRef<Zep::RangeMarker>(*tip_msg->pBuffer);

      if (syntax->GetSyntaxAt(tip_msg->location).foreground == Zep::ThemeColor::Identifier) {
        marker->SetDescription("Identifier");
        marker->SetHighlightColor(Zep::ThemeColor::Identifier);
      } else if (syntax->GetSyntaxAt(tip_msg->location).foreground == Zep::ThemeColor::Keyword) {
        marker->SetDescription("Keyword");
        marker->SetHighlightColor(Zep::ThemeColor::Keyword);
      }
        
      marker->SetTextColor(Zep::ThemeColor::Text);
      tip_msg->spMarker = marker;
      tip_msg->handled = true;
    }
  }

} // namespace other
