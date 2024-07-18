/**
 * \file ecs/systems/component_gui.cpp
 **/
#include "ecs/systems//component_gui.hpp"

#include <imgui/imgui.h>

#include "ecs/components/mesh.hpp"
#include "event/event_queue.hpp"
#include "event/app_events.hpp"

#include "application/app_state.hpp"

#include "rendering/camera_base.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/orthographic_camera.hpp"
#include "rendering/model.hpp"
#include "rendering/model_factory.hpp"
#include "rendering/ui/ui_helpers.hpp"
#include "rendering/ui/ui_widgets.hpp"

#include "ecs/components/transform.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/rigid_body_2d.hpp"

namespace other {
  
  void DrawTag(Entity* ent) {
    const auto& tag = ent->GetComponent<Tag>();
    ImGui::Text("ID : %llu" , tag.id.Get());
    ImGui::Text("Name : %s" , tag.name.c_str());
  }

  void DrawTransform(Entity *ent) {
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
      auto& component = ent->GetComponent<Transform>();
      
      ImGui::TableNextRow();
      ui::widgets::DrawVec3Control("Translation" , component.position , translation_manually_edited , 0.f , /// replace this value from redo/undo stack
                                    100.f , ui::VectorAxis::ZERO , glm::zero<glm::vec3>() , glm::zero<glm::vec3>() , 0.1f);

      ImGui::TableNextRow();
      if (ui::widgets::DrawVec3Control("Rotation" , component.erotation , rotation_manually_edited , 0.f , /// replace this value from redo/undo stack
                                    100.f , ui::VectorAxis::ZERO , glm::zero<glm::vec3>() , glm::zero<glm::vec3>() , 0.1f)) {
        component.qrotation = glm::quat(component.erotation);
      }

      ImGui::TableNextRow();
      ui::widgets::DrawVec3Control("Scale" , component.scale , scale_manually_edited , 1.f , /// replace this value from redo/undo stack
                                    100.f , ui::VectorAxis::ZERO , glm::zero<glm::vec3>() , glm::zero<glm::vec3>() , 0.1f);
      
      if (translation_manually_edited || rotation_manually_edited || scale_manually_edited) {
        component.CalcMatrix();
        OE_DEBUG("Transform calculated : {}" , component.model_transform);
      }
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

    // ImGui::Text("Put options related to how parents and children interact");
    // ImGui::Text(" > have children inherit parent position? ");
    // ImGui::Text(" > other things...");
  }

  template <typename T>
  bool DrawFieldValue(ScriptField* field , ScriptObject* script_instance) {
    OE_ASSERT(field != nullptr , "Attempting to draw null script field");

    bool result = false;

    T v = field->value.Read<T>();
    T min = 0 , max = 0;
    if (field->bounds.has_value()) {
      min = field->bounds->x;
      max = field->bounds->y;
    }

    if (ui::Property(field->name.c_str() , &v , min , max)) {
      field->value.Set(v); 
      script_instance->SetField(field->name , field->value);
      result = true;
    }

    return result;
  }

  template <>
  bool DrawFieldValue<bool>(ScriptField* field , ScriptObject* script_instance) {
    bool result = false;

    bool v = field->value.Get<bool>();
    
    ui::BeginProperty(field->name.c_str());
    if (ImGui::Checkbox("" , &v)) {
      field->value.Set(v);
      script_instance->SetField(field->name, field->value);
      result = true;
    }
    ui::EndProperty();

    return result;
  }

  template <>
  bool DrawFieldValue<char>(ScriptField* field , ScriptObject* script_instance) {
    bool result = false;

    char v = field->value.Get<char>();
    static std::array<char , 52> chars = {
      'a' , 'b' , 'c' , 'd' , 'e' , 'f' , 'g' , 'h' , 'i' , 'j' , 'k' , 'l' , 'm' , 'n' , 
            'o' , 'p' , 'q' , 'r' , 's' , 't' , 'u' , 'v' , 'w' , 'x' , 'y' , 'z' ,
      'A' , 'B' , 'C' , 'D' , 'E' , 'F' , 'G' , 'H' , 'I' , 'J' , 'K' , 'L' , 'M' , 'N' ,
            'O' , 'P' , 'Q' , 'R' , 'S' , 'T' , 'U' , 'V' , 'W' , 'X' , 'Y' , 'Z'
    };
    
    ui::BeginProperty(field->name.c_str());
    
    if (ImGui::BeginCombo(("##char-dropdown" + field->name).c_str() , &v)) {
      for (auto& c : chars) {
        bool is_selected = v == c;
        std::string char_str{ c };
        if (ImGui::Selectable(char_str.c_str() , is_selected)) {
          field->value.Set<char>(c);
          script_instance->SetField(field->name , field->value);
          result = true;
        }
      }
      ImGui::EndCombo();
    }

    ui::EndProperty();

    return result;
  }

  template <> 
  bool DrawFieldValue<std::string>(ScriptField* field , ScriptObject* script_instance) {
    bool result = false;

    std::string v = field->value.Get<std::string>();
    std::array<char , 256> buffer{};
    memset(buffer.data() , 0 , 256);
    memcpy(buffer.data() , v.c_str() , v.length());

    ui::BeginProperty(field->name.c_str());

    if (ImGui::InputText(("##script-string-" + field->name).c_str() , buffer.data() , buffer.size())) {
      if (ImGui::IsKeyPressed(ImGuiKey_Enter) && buffer[0] != 0) {
        v = buffer.data();
        field->value.SetStr(v);
        script_instance->SetField(field->name , field->value);
        result = true;
      }
    }

    ui::EndProperty();

    return result;
  }

  template <>
  bool DrawFieldValue<glm::vec2>(ScriptField* field , ScriptObject* script_instance) {
    bool result = false;

    bool modified = false;
    
    float min = 0 , max = 0 , speed = 1.f;
    if (field->bounds.has_value()) {
      min = field->bounds->x;
      max = field->bounds->y;

      min = glm::min(min , max);
      max = glm::max(min , max);

      speed = (max - min) / 20.f;
    }
    
    ui::BeginProperty(field->name.c_str());

    glm::vec2 v = field->value.Get<glm::vec2>();
    if (ui::widgets::DrawVec2Control(field->name , v , modified , 0.f , 100.f , ui::VectorAxis::ZERO ,
                                      glm::vec2(min) , glm::vec2(max) , speed)) {
      field->value.Set(v);
      script_instance->SetField(field->name , field->value);
      result = true;
    }
    
    ui::EndProperty();

    return result;
  }
  
  template <>
  bool DrawFieldValue<glm::vec3>(ScriptField* field , ScriptObject* script_instance) {
    bool result = false;

    bool modified = false;

    float min = 0 , max = 0 , speed = 1.f;
    if (field->bounds.has_value()) {
      min = field->bounds->x;
      max = field->bounds->y;

      min = glm::min(min , max);
      max = glm::max(min , max);

      speed = (max - min) / 20.f;
    }

    ui::BeginProperty(field->name.c_str());

    glm::vec3 v = field->value.Get<glm::vec3>();
    if (ui::widgets::DrawVec3Control("" , v , modified , 0.f , 100.f , ui::VectorAxis::ZERO ,
                                      glm::vec3(min) , glm::vec3(max) , speed)) {
      field->value.Set(v);
      script_instance->SetField(field->name , field->value);
      result = true;
    }

    ui::EndProperty();

    return result;
  }
  
  template <>
  bool DrawFieldValue<glm::vec4>(ScriptField* field , ScriptObject* script_instance) {
    bool result = false;

    bool modified = false;
    
    float min = 0 , max = 0 , speed = 1.f;
    if (field->bounds.has_value()) {
      min = field->bounds->x;
      max = field->bounds->y;

      min = glm::min(min , max);
      max = glm::max(min , max);

      speed = (max - min) / 20.f;
    }
    
    ui::BeginProperty(field->name.c_str());

    glm::vec4 v = field->value.Get<glm::vec4>();
    if (ui::widgets::DrawVec4Control(field->name , v , modified , 0.f , 100.f , ui::VectorAxis::ZERO ,
                                      glm::vec4(min) , glm::vec4(max) , speed)) {
      field->value.Set(v);
      script_instance->SetField(field->name , field->value);
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
      case ValueType::BOOL: result = DrawFieldValue<bool>(field , script_instance); break;
      case ValueType::CHAR: result = DrawFieldValue<char>(field , script_instance); break;
      case ValueType::INT8: result = DrawFieldValue<int8_t>(field , script_instance); break;
      case ValueType::UINT8: result = DrawFieldValue<uint8_t>(field , script_instance); break;
      case ValueType::INT16: result = DrawFieldValue<int16_t>(field , script_instance); break;
      case ValueType::UINT16: result = DrawFieldValue<uint16_t>(field , script_instance); break;
      case ValueType::INT32: result = DrawFieldValue<int32_t>(field , script_instance); break;
      case ValueType::UINT32: result = DrawFieldValue<uint32_t>(field , script_instance); break;
      case ValueType::INT64: result = DrawFieldValue<int64_t>(field , script_instance); break;
      case ValueType::UINT64: result = DrawFieldValue<uint32_t>(field , script_instance); break;
      case ValueType::FLOAT: result = DrawFieldValue<float>(field , script_instance); break;
      case ValueType::DOUBLE: result = DrawFieldValue<double>(field , script_instance); break;
      case ValueType::VEC2: result = DrawFieldValue<glm::vec2>(field , script_instance); break;
      case ValueType::VEC3: result = DrawFieldValue<glm::vec3>(field , script_instance); break;
      case ValueType::VEC4: result = DrawFieldValue<glm::vec4>(field , script_instance); break;
      case ValueType::STRING: result = DrawFieldValue<std::string>(field , script_instance); break;
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

    std::vector<UUID> scripts_to_remove;
 
    for (auto& [id , s] : script.scripts) {
      ImGui::PushID(id.Get());

      bool is_error = s->IsCorrupt();
      if (is_error) {
        ImGui::Text("%s corrupt, recompile or reload" , s->Name().c_str());

        if (ImGui::Button("Rebuild Scripts")) {
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

      if (ImGui::Button("Remove Script")) {
        scripts_to_remove.push_back(id);
      }
      
      ImGui::PopItemWidth();

      ui::EndPropertyGrid();
      ui::Underline();
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

    for (const auto& id : scripts_to_remove) {
      script.scripts.erase(script.scripts.find(id));
      script.data.erase(script.data.find(id));
    }
  }
  
  void DrawMesh(Entity* ent) {
    auto& mesh = ent->GetComponent<Mesh>();

    if (mesh.handle == 0) {
      /// display other meshes
      return;
    } 

    Ref<Model> model = AppState::Assets()->GetAsset(mesh.handle);
    if (model == nullptr) {
      ScopedColor red_text(ImGuiCol_Text , ui::theme::red);
      ImGui::Text("Mesh [{}] invalid");
      return;
    }

    ImGui::Text("Rendering Mesh");
  }
  
  void DrawStaticMesh(Entity* ent) {
    auto& mesh = ent->GetComponent<StaticMesh>();

    const char* options[] = {
      "Empty" , "Triangle" , "Rect" , "Cube" , "Sphere" , "Capsule"
    };

    if (ui::PropertyDropdown("Primitive Meshes" , options, kCapsuleIdx , mesh.primitive_selection)) {}

    if (mesh.primitive_id != mesh.primitive_selection && ImGui::Button("Confirm Change")) {
      bool change = false;
      switch (mesh.primitive_selection) {
        case kTriangleIdx:
          OE_WARN("Assigning rect until implementations for others available!");
          [[ fallthrough ]];
        case kRectIdx: {
          auto& scale = ent->ReadComponent<Transform>().scale;
          auto& pos = ent->ReadComponent<Transform>().position;
          mesh.handle = ModelFactory::CreateRect(pos , { scale.x / 2 , scale.y / 2 });
          if (!AppState::Assets()->IsValid(mesh.handle)) {
            OE_ERROR("Failed to Create Rect Static Mesh");
          } else {
            change = true; 
          }
        } break;

        case kSphereIdx:
        case kCapsuleIdx:
          OE_WARN("Assigning cube until implementations for others available!");
          [[ fallthrough ]];
        case kCubeIdx: {
          auto& scale = ent->ReadComponent<Transform>().scale;
          mesh.handle = ModelFactory::CreateBox(scale);
          if (!AppState::Assets()->IsValid(mesh.handle)) {
            OE_ERROR("Failed to Create Cube Static Mesh");
          } else {
            change = true; 
          }
        } break;

        case kEmptyIdx:
        default:
          break;
      }

      if (change) {
        mesh.primitive_id = mesh.primitive_selection;
        if (mesh.primitive_id != kEmptyIdx) {
          mesh.is_primitive = true;
        }
      }
    }

    if (mesh.primitive_id == kEmptyIdx) {
      return;
    }
      
    bool valid = AppState::Assets()->IsHandleValid(mesh.handle);
    if (!valid) {
      ScopedColor red_text(ImGuiCol_Text , ui::theme::red);
      ImGui::Text("Static Mesh handle [%lld] invalid" , mesh.handle.Get());
      return;
    }

    valid = AppState::Assets()->IsValid(mesh.handle);
    if (!valid) {
      ScopedColor red_text(ImGuiCol_Text , ui::theme::red);
      ImGui::Text("Static Mesh [{}] invalid");
      return;
    }

    Ref<StaticModel> model = AppState::Assets()->GetAsset(mesh.handle);
    if (model == nullptr) {
      ScopedColor red_text(ImGuiCol_Text , ui::theme::red);
      ImGui::Text("Unknown error retrievieng Static Mesh [{}] from asset handler");
      return;
    }

    //// draw selection and settings 

    ImGui::Text("Rendering Mesh");
  }
  
  void DrawCamera(Entity* ent) {
    auto& camera = ent->GetComponent<Camera>();

    ui::BeginPropertyGrid();

    const char* proj_types[] = {
      "Perspective" ,
      "Orthographic"
    };

    bool primary = camera.camera->IsPrimary();
    if (ImGui::Checkbox("Primary Camera" , &primary)) {
      camera.camera->SetPrimary(primary);
    }

    uint32_t current_proj = camera.camera->GetCameraProjectionType();
    
    if (ui::PropertyDropdown("Projection" , proj_types , 2 , current_proj)) {
      switch (current_proj) {
        case CameraProjectionType::PERSPECTIVE:
          camera.camera = Ref<PerspectiveCamera>::Create(camera.camera);
        break;

        case CameraProjectionType::ORTHOGRAPHIC:
          camera.camera = Ref<OrthographicCamera>::Create(camera.camera);
        break;
        
        default:
          break;
      }
    }

    bool position_modified = false;
    bool orientation_modified = false;

    ui::ShiftCursorY(4.5f);
    ImGui::Checkbox("Pinned to Object Position" , &camera.pinned_to_entity_position); 

    ImGui::NextColumn();

    if (!camera.pinned_to_entity_position) {
      ui::widgets::DrawVec3Control("Position" , camera.camera->position , position_modified);
      ui::widgets::DrawVec3Control("Orientation" , camera.camera->euler_angles , orientation_modified);
    }

    ImGui::NextColumn();

    switch(camera.camera->GetCameraProjectionType()) {
      case CameraProjectionType::PERSPECTIVE: {
        ui::DrawTodoReminder("PERSPECTIVE CAMERA OPTIONS"); 
      } break;

      case CameraProjectionType::ORTHOGRAPHIC: {
        ui::DrawTodoReminder("ORTHOGRAPHIC CAMERA OPTIONS");
      } break;


      default:
        ScopedColor col(ImGuiCol_Text , ui::theme::red);
        ImGui::Text("Invalid Camera Projection Type! Camera Corrupt");
      break;
    }

    ui::EndPropertyGrid();
  }
  
  void DrawRigidBody2D(Entity* ent) {
    ui::BeginPropertyGrid();

    auto& body = ent->GetComponent<RigidBody2D>();

    const char* body_type_strings[] = {
      "Static" , "Kinematic" , "Dynamic"
    };

    uint32_t selected = body.type;
    if (selected >= INVALID_PHYSICS_BODY) {
      ScopedColor red(ImGuiCol_Text , ui::theme::red);
      ImGui::Text("Invalid valid for Rigid Body 2D body type : %d" , body.type);
    } else {
      if (ui::PropertyDropdown("Type" , body_type_strings , 3 , selected)) {
        body.type = static_cast<PhysicsBodyType>(selected);
        ent->UpdateComponent<RigidBody2D>(body);
      }
      
      if (body.type == PhysicsBodyType::DYNAMIC) {
        ui::BeginPropertyGrid();
        
        ui::Property("Mass" , &body.mass);
        ui::Property("Linear Drag" , &body.linear_drag);
        ui::Property("Angular Drag" , &body.angular_drag);
        ui::Property("Gravity Scale" , &body.gravity_scale);
        ui::Property("Fixed Rotation" , &body.fixed_rotation);
        ui::Property("Bullet" , &body.bullet);

        ui::EndPropertyGrid();
      } 
    }

    ui::EndPropertyGrid();
  }
  
  void DrawCollider2D(Entity* ent) {
    ui::BeginPropertyGrid();

    ui::EndPropertyGrid();
  }

} // namespace other
