/**
 * \file ecs/systems/component_gui.cpp
 **/
#include "ecs/systems//component_gui.hpp"

#include <imgui/imgui.h>

#include "event/event_queue.hpp"
#include "event/app_events.hpp"
#include "rendering/ui/ui_helpers.hpp"
#include "rendering/ui/ui_widgets.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/script.hpp"

namespace other {

  void DrawTransform(Entity *entity) {
    ScopedStyle spacing(ImGuiStyleVar_ItemSpacing , ImVec2(8.f , 8.f));
    ScopedStyle padding(ImGuiStyleVar_FramePadding , ImVec2(4.f , 4.f));

    ImGui::BeginTable("Transform Component" , 2 , ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoClip);
    ImGui::TableSetupColumn("label_col" , 0 , 100.f);
    ImGui::TableSetupColumn("value_col" , ImGuiTableColumnFlags_IndentEnable | ImGuiTableColumnFlags_NoClip , ImGui::GetContentRegionAvail().x - 100.f);

    bool translation_manually_edited = false;
    bool rotation_manually_edited = false;
    bool scale_manually_edited = false;

    /// replace with this input parameter
    bool multi_edit = false;
    if (multi_edit) {
    } else {
      auto& component = entity->GetComponent<Transform>();
      
      ImGui::TableNextRow();
      ui::widgets::DrawVec3Control("Translation" , component.position , translation_manually_edited);

      ImGui::TableNextRow();
      if (ui::widgets::DrawVec3Control("Rotation" , component.erotation , rotation_manually_edited)) {
        /// recalculate quaternion
      }

      ImGui::TableNextRow();
      ui::widgets::DrawVec3Control("Scale" , component.scale , scale_manually_edited , 1.f);
    }

    ImGui::EndTable();

    ui::ShiftCursorY(-8.f);
    ui::Underline();

    ui::ShiftCursorY(18.f);
  }
  
  void DrawRelationship(Entity* ent) {
    auto& relations = ent->GetComponent<Relationship>();

    if (!relations.parent.has_value()) {
      ImGui::Text("Root Entity");
    } else {
      ImGui::Text("Parent Entity > [ %lld ]" , relations.parent.value().Get());
    }

    ImGui::Text("Put options related to how parents and children interact");
    ImGui::Text(" > have children inherit parent position? ");
    ImGui::Text(" > other things...");
  }

  template <typename T>
  bool DrawFieldValue(const std::string& name , Value& val , ScriptObject* script_instance) {
    bool result = false;

    T v = val.Read<T>();
    if (ui::Property(name.c_str() , &v)) {
      val.Set(v); 
      script_instance->SetField(name , val);
      result = true;
    }

    return result;
  }

  template <>
  bool DrawFieldValue<bool>(const std::string& name , Value& val , ScriptObject* script_instance) {
    bool result = false;

    bool v = val.Get<bool>();
    
    ui::BeginProperty(name.c_str());
    if (ImGui::Checkbox("" , &v)) {
      val.Set(v);
      script_instance->SetField(name, val);
      result = true;
    }
    ui::EndProperty();

    return result;
  }

  template <>
  bool DrawFieldValue<char>(const std::string& name , Value& val , ScriptObject* script_instance) {
    bool result = false;

    char v = val.Get<char>();
    static std::array<char , 52> chars = {
      'a' , 'b' , 'c' , 'd' , 'e' , 'f' , 'g' , 'h' , 'i' ,'j' , 'k' , 'l' , 'm' , 'n' , 
            'o' , 'p' , 'q' , 'r' , 's' , 't' , 'u' , 'v' , 'w' , 'x' , 'y' , 'z' ,
      'A' , 'B' , 'C' , 'D' , 'E' , 'F' , 'G' , 'H' , 'I' , 'J' , 'K' , 'L' , 'M' , 'N' ,
            'O' , 'P' , 'Q' , 'R' , 'S' , 'T' , 'U' , 'V' , 'W' , 'X' , 'Y' , 'Z'
    };
    
    ui::BeginProperty(name.c_str());
    
    if (ImGui::BeginCombo(("##char-dropdown" + name).c_str() , &v)) {
      for (auto& c : chars) {
        bool is_selected = v == c;
        std::string char_str{ c };
        if (ImGui::Selectable(char_str.c_str() , is_selected)) {
          val.Set<char>(c);
          script_instance->SetField(name , val);
          result = true;
        }
      }
      ImGui::EndCombo();
    }

    ui::EndProperty();

    return result;
  }

  template <> 
  bool DrawFieldValue<std::string>(const std::string& name , Value& val , ScriptObject* script_instance) {
    bool result = false;

    std::string v = val.Get<std::string>();
    std::array<char , 256> buffer{};
    memset(buffer.data() , 0 , 256);
    memcpy(buffer.data() , v.c_str() , v.length());

    ui::BeginProperty(name.c_str());

    if (ImGui::InputText(("##script-string-" + name).c_str() , buffer.data() , buffer.size())) {
      v = buffer.data();
      val.SetStr(v);
      script_instance->SetField(name , val);
      result = true;
    }

    ui::EndProperty();

    return result;
  }

  void DrawScriptArray(ScriptField* array , ScriptObject* script_instance) {
    // ImGui::Text("Script array : %s" , array->name.c_str()); 
  }

  void DrawScriptField(ScriptField* field , ScriptObject* script_instance) {
    bool result = false;

    switch (field->value.Type()) {
      case ValueType::BOOL: result = DrawFieldValue<bool>(field->name , field->value , script_instance); break;
      case ValueType::CHAR: result = DrawFieldValue<char>(field->name , field->value , script_instance); break;
      case ValueType::INT8: result = DrawFieldValue<int8_t>(field->name , field->value , script_instance); break;
      case ValueType::UINT8: result = DrawFieldValue<uint8_t>(field->name , field->value , script_instance); break;
      case ValueType::INT16: result = DrawFieldValue<int16_t>(field->name , field->value , script_instance); break;
      case ValueType::UINT16: result = DrawFieldValue<uint16_t>(field->name , field->value , script_instance); break;
      case ValueType::INT32: result = DrawFieldValue<int32_t>(field->name , field->value , script_instance); break;
      case ValueType::UINT32: result = DrawFieldValue<uint32_t>(field->name , field->value , script_instance); break;
      case ValueType::INT64: result = DrawFieldValue<int64_t>(field->name , field->value , script_instance); break;
      case ValueType::UINT64: result = DrawFieldValue<uint32_t>(field->name , field->value , script_instance); break;
      case ValueType::FLOAT: result = DrawFieldValue<float>(field->name , field->value , script_instance); break;
      case ValueType::DOUBLE: result = DrawFieldValue<double>(field->name , field->value , script_instance); break;
      case ValueType::VEC2: result = DrawFieldValue<glm::vec2>(field->name , field->value , script_instance); break;
      case ValueType::VEC3: result = DrawFieldValue<glm::vec3>(field->name , field->value , script_instance); break;
      case ValueType::VEC4: result = DrawFieldValue<glm::vec4>(field->name , field->value , script_instance); break;
      case ValueType::STRING: result = DrawFieldValue<std::string>(field->name , field->value , script_instance); break;
      case ValueType::ASSET: 
        ImGui::Text("Asset values unimplemented!");
      break;
      case ValueType::ENTITY: 
        ImGui::Text("Entity values unimplemented!");
      break;
      case ValueType::USER_TYPE:
        ImGui::Text("User Types unimplemented");
      break;
      default:
        ImGui::Text("Field value %s has corrupted type" , field->name.c_str());
    }
  }

  static Opt<Value> test_value;

  void DrawScript(Entity* ent) {
    auto& script = ent->GetComponent<Script>();
 
    for (auto& [id , s] : script.scripts) {
      ImGui::PushID(id.Get());

      bool is_error = s->IsCorrupt();
      if (is_error) {
        ImGui::Text("%s corrupt, recompile or reload" , s->Name().c_str());

        if (ImGui::Button(("Reload Script##" + s->Name()).c_str())) {
          EventQueue::PushEvent<ScriptReloadEvent>();
        }
        return;
      }
    
      ui::BeginPropertyGrid();
      ui::ShiftCursor(10.f , 9.f);
      std::string lang_name = s->LanguageType() == LanguageModuleType::CS_MODULE ? "C#" : "Lua";
      ImGui::Text("%s [ %s ]" , s->Name().c_str() , lang_name.c_str());

      ImGui::NextColumn();
      ui::ShiftCursorY(4.f);
      ImGui::PushItemWidth(-1);

      ImVec2 og_button_txt_align = ImGui::GetStyle().ButtonTextAlign;
      {
        ImGui::GetStyle().ButtonTextAlign = { 0.f , 0.5f };
        float width = ImGui::GetContentRegionAvail().x;
        float item_height = 28.f;

        std::string button_txt = "None";
        /// get full name
        /// button_txt = full_name

        if ((GImGui->CurrentItemFlags & ImGuiItemFlags_MixedValue) != 0) {
          button_txt = "---";
        }

        {
          ScopedColor bg_col(ImGuiCol_WindowBg , ui::theme::property_field);
          ScopedColor button_label_col(ImGuiCol_Text , ui::theme::text);
          ImGui::Button(button_txt.c_str() , { width , item_height });

          const bool hovered = ImGui::IsItemHovered();

          if (hovered) {
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
              /// open script in text editor
            } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
              ImGui::OpenPopup(("##script_popup" + s->Name()).c_str());
            }
          }
        }
      }

      ImGui::GetStyle().ButtonTextAlign = og_button_txt_align;
      bool clear = false; 
      if (ImGui::BeginPopup(("##script_popup" + s->Name()).c_str())) {
        if (clear) {
           
        }

        ImGui::EndPopup();
      }

      ImGui::PopItemWidth();
      ui::EndPropertyGrid();

      ui::BeginPropertyGrid();
      
      /// display exported properties
      for (auto& [field_id , val] : s->GetFields()) {
        ImGui::PushID(("##script-field" + val.name).c_str());
        ImGui::SetCursorPosX(50.f);

        if (val.value.IsArray()) {
          DrawScriptArray(&val , s);
        } else {
          DrawScriptField(&val , s);
        } 

        ImGui::PopID();
      }

      ui::EndPropertyGrid();

      ImGui::PopID();
    }

  }
  
  void DrawMesh(Entity* ent) {}

} // namespace other
