/**
 * \file ecs/systems/component_gui.cpp
 **/
#include "ecs/systems/component_gui.hpp"

#include <glm/ext/vector_float4.hpp>
#include <imgui/imgui.h>

#include "core/defines.hpp"
#include "core/filesystem.hpp"

#include "application/app_state.hpp"
#include "asset/asset_defines.hpp"
#include "asset/asset_manager.hpp"
#include "event/core_events.hpp"
#include "event/event_queue.hpp"

#include "ecs/components/camera.hpp"
#include "ecs/components/collider_2d.hpp"
#include "ecs/components/light_source.hpp"
#include "ecs/components/physics_component.hpp"
#include "ecs/components/rigid_body_2d.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/transform.hpp"
#include "scene/scene_manager.hpp"

#include "physics/3D/physics_shape.hpp"
#include "rendering/camera_base.hpp"
#include "rendering/model.hpp"
#include "rendering/model_factory.hpp"
#include "rendering/orthographic_camera.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/ui/ui_colors.hpp"
#include "rendering/ui/ui_helpers.hpp"
#include "rendering/ui/ui_widgets.hpp"
#include "scripting/script_engine.hpp"

namespace other {

  bool DrawTransform(Entity* ent) {
    ScopedStyle spacing(ImGuiStyleVar_ItemSpacing, ImVec2(8.f, 8.f));
    ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(4.f, 4.f));

    bool modified = false;

    ImGui::BeginTable("Transform Component", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoClip);
    ImGui::TableSetupColumn("label_col", 0, 100.f);
    ImGui::TableSetupColumn("value_col", ImGuiTableColumnFlags_IndentEnable | ImGuiTableColumnFlags_NoClip, ImGui::GetContentRegionAvail().x - 100.f);

    bool translation_manually_edited = false;
    bool rotation_manually_edited = false;
    bool scale_manually_edited = false;

    /// replace with this input parameter
    bool multi_edit = false;
    if (multi_edit) {
    } else {
      Transform& component = ent->GetComponent<Transform>();

      ImGui::TableNextRow();
      if (ui::widgets::DrawVec3Control("Translation", component.position, translation_manually_edited, 0.f,  /// replace this value from redo/undo stack
                                       100.f, ui::VectorAxis::ZERO, glm::zero<glm::vec3>(), glm::zero<glm::vec3>(), 0.1f)) {
      }

      ImGui::TableNextRow();
      if (ui::widgets::DrawVec3Control("Rotation", component.erotation, rotation_manually_edited, 0.f,  /// replace this value from redo/undo stack
                                       100.f, ui::VectorAxis::ZERO, glm::zero<glm::vec3>(), glm::zero<glm::vec3>(), 0.1f)) {
        component.qrotation = glm::quat(component.erotation);
      }

      ImGui::TableNextRow();
      if (ui::widgets::DrawVec3Control("Scale", component.scale, scale_manually_edited, 1.f,  /// replace this value from redo/undo stack
                                       100.f, ui::VectorAxis::ZERO, glm::zero<glm::vec3>(), glm::zero<glm::vec3>(), 0.1f)) {
      }

      if (translation_manually_edited || rotation_manually_edited || scale_manually_edited) {
        component.CalcMatrix();
        modified = true;

        if (ent->HasComponent<RigidBody>()) {
          auto& body = ent->GetComponent<RigidBody>();
          body.physics_body->SetTransform(component);
        }

        if (ent->HasComponent<Collider>()) {
          auto& collider = ent->GetComponent<Collider>();
          collider.shape->SetTransform(component);
        }
      }
    }

    ImGui::EndTable();

    ui::ShiftCursorY(-8.f);
    ui::Underline();

    ui::ShiftCursorY(18.f);

    if (modified) {
      /// get active scene and update bounding boxes
      auto* md = AppState::Scenes()->ActiveScene();
      OE_ASSERT(md != nullptr, "No active scene found!");
      OE_ASSERT(md->bvh != nullptr, "No bounding hierarchy for active scene!");

      md->bvh->Update();
    }

    return modified;
  }

  template <typename T>
  bool DrawFieldValue(ScriptField* field, Script& script_instance) {
    OE_ASSERT(field != nullptr, "Attempting to draw null script field");

    bool result = false;

    T v = field->value.Get<T>();
    T min = 0, max = 0;
    if (field->bounds.has_value()) {
      min = field->bounds->x;
      max = field->bounds->y;
    }

    if (ui::Property(field->name.c_str(), &v, min, max)) {
      field->value = v;
      script_instance.SetField(field->name, field->value);
      result = true;
    }

    return result;
  }

  template <>
  bool DrawFieldValue<bool>(ScriptField* field, Script& script_instance) {
    bool result = false;

    bool v = field->value.Get<bool>();

    ui::BeginProperty(field->name.c_str());
    if (ImGui::Checkbox("", &v)) {
      // field->value.Set(v);
      // script_instance->SetField(field->name, field->value);
      // result = true;
    }
    ui::EndProperty();

    return result;
  }

  template <>
  bool DrawFieldValue<char>(ScriptField* field, Script& script_instance) {
    bool result = false;

    // char v = field->value.Get<char>();
    // static std::array<char, 52> chars = {
    //   'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    //   'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    //   'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    //   'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
    // };

    // ui::BeginProperty(field->name.c_str());

    // if (ImGui::BeginCombo(("##char-dropdown" + field->name).c_str(), &v)) {
    //   for (auto& c : chars) {
    //     bool is_selected = v == c;
    //     std::string char_str{ c };
    //     if (ImGui::Selectable(char_str.c_str(), is_selected)) {
    //       field->value.Set<char>(c);
    //       script_instance->SetField(field->name, field->value);
    //       result = true;
    //     }
    //   }
    //   ImGui::EndCombo();
    // }

    ui::EndProperty();

    return result;
  }

  template <>
  bool DrawFieldValue<std::string>(ScriptField* field, Script& script_instance) {
    bool result = false;

    // std::string v = field->value.Get<std::string>();
    // std::array<char, 256> buffer{};
    // memset(buffer.data(), 0, 256);
    // memcpy(buffer.data(), v.c_str(), v.length());

    // ui::BeginProperty(field->name.c_str());

    // if (ImGui::InputText(("##script-string-" + field->name).c_str(), buffer.data(), buffer.size())) {
    //   if (ImGui::IsKeyPressed(ImGuiKey_Enter) && buffer[0] != 0) {
    //     v = buffer.data();
    //     field->value.Set<std::string>(v);
    //     script_instance->SetField(field->name, field->value);
    //     result = true;
    //   }
    // }

    // ui::EndProperty();

    return result;
  }

  template <>
  bool DrawFieldValue<glm::vec2>(ScriptField* field, Script& script_instance) {
    bool result = false;

    // bool modified = false;

    // float min = 0, max = 0, speed = 1.f;
    // if (field->bounds.has_value()) {
    //   min = field->bounds->x;
    //   max = field->bounds->y;

    //   min = glm::min(min, max);
    //   max = glm::max(min, max);

    //   speed = (max - min) / 20.f;
    // }

    // ui::BeginProperty(field->name.c_str());

    // glm::vec2 v = field->value.Get<glm::vec2>();
    // if (ui::widgets::DrawVec2Control(field->name, v, modified, 0.f, 100.f, ui::VectorAxis::ZERO, glm::vec2(min), glm::vec2(max), speed)) {
    //   field->value.Set(v);
    //   script_instance->SetField(field->name, field->value);
    //   result = true;
    // }

    // ui::EndProperty();

    return result;
  }

  template <>
  bool DrawFieldValue<glm::vec3>(ScriptField* field, Script& script_instance) {
    bool result = false;

    bool modified = false;

    float min = 0, max = 0, speed = 1.f;
    if (field->bounds.has_value()) {
      min = field->bounds->x;
      max = field->bounds->y;

      min = glm::min(min, max);
      max = glm::max(min, max);

      speed = (max - min) / 20.f;
    }

    ui::BeginProperty(field->name.c_str());

    glm::vec3 v = field->value.Get<glm::vec3>();
    if (ui::widgets::DrawVec3Control("", v, modified, 0.f, 100.f, ui::VectorAxis::ZERO, glm::vec3(min), glm::vec3(max), speed)) {
      // field->value.Set(v);
      // script_instance->SetField(field->name, field->value);
      // result = true;
    }

    ui::EndProperty();

    return result;
  }

  template <>
  bool DrawFieldValue<glm::vec4>(ScriptField* field, Script& script_instance) {
    bool result = false;

    bool modified = false;

    float min = 0, max = 0, speed = 1.f;
    if (field->bounds.has_value()) {
      min = field->bounds->x;
      max = field->bounds->y;

      min = glm::min(min, max);
      max = glm::max(min, max);

      speed = (max - min) / 20.f;
    }

    ui::BeginProperty(field->name.c_str());

    glm::vec4 v = field->value.Get<glm::vec4>();
    if (ui::widgets::DrawVec4Control(field->name, v, modified, 0.f, 100.f, ui::VectorAxis::ZERO, glm::vec4(min), glm::vec4(max), speed)) {
      // field->value.Set(v);
      // script_instance->SetField(field->name, field->value);
      // result = true;
    }

    ui::EndProperty();

    return result;
  }

  void DrawField(ScriptField* field, Script& script_instance) {
    OE_ASSERT(field != nullptr, "Attempting to draw null script field");

    std::string label = field->name;
    ui::BeginProperty(field->name.c_str(), "<description>");
    std::string type_name = std::string{ ValueTypeToString(field->value.GetType()) };
    ImGui::Text("Field type : [%s]", type_name.c_str());
    ui::EndProperty();
  }

  bool DrawScriptArray(ScriptField* array, Script& script_instance) {
    // ImGui::Text("Script array : %s" , array->name.c_str());

    return false;
  }

  bool DrawScriptField(ScriptField* field, Script& script_instance) {
    bool result = false;

    switch (field->value.GetType()) {
      case ValueType::BOOL:
        result = result &= DrawFieldValue<bool>(field, script_instance);
        break;
      case ValueType::CHAR:
        result = result &= DrawFieldValue<char>(field, script_instance);
        break;
      case ValueType::INT8:
        result = result &= DrawFieldValue<int8_t>(field, script_instance);
        break;
      case ValueType::UINT8:
        result = result &= DrawFieldValue<uint8_t>(field, script_instance);
        break;
      case ValueType::INT16:
        result = result &= DrawFieldValue<int16_t>(field, script_instance);
        break;
      case ValueType::UINT16:
        result = result &= DrawFieldValue<uint16_t>(field, script_instance);
        break;
      case ValueType::INT32:
        result = result &= DrawFieldValue<int32_t>(field, script_instance);
        break;
      case ValueType::UINT32:
        result = result &= DrawFieldValue<uint32_t>(field, script_instance);
        break;
      case ValueType::INT64:
        result = result &= DrawFieldValue<int64_t>(field, script_instance);
        break;
      case ValueType::UINT64:
        result = result &= DrawFieldValue<uint32_t>(field, script_instance);
        break;
      case ValueType::FLOAT:
        result = result &= DrawFieldValue<float>(field, script_instance);
        break;
      case ValueType::DOUBLE:
        result = result &= DrawFieldValue<double>(field, script_instance);
        break;
      case ValueType::VEC2:
        result = result &= DrawFieldValue<glm::vec2>(field, script_instance);
        break;
      case ValueType::VEC3:
        result = result &= DrawFieldValue<glm::vec3>(field, script_instance);
        break;
      case ValueType::VEC4:
        result = result &= DrawFieldValue<glm::vec4>(field, script_instance);
        break;
      case ValueType::STRING:
        result = result &= DrawFieldValue<std::string>(field, script_instance);
        break;
      case ValueType::ASSET:
      case ValueType::ENTITY:
      case ValueType::USER_TYPE:
      default:
        DrawField(field, script_instance);
        break;
    }

    return result;
  }

  bool DrawScript(Entity* ent) {
    // const std::vector<ScriptObjectTag>& loaded_script_objs = ScriptEngine::GetLoadedObjects();
    // if (loaded_script_objs.empty()) {
    //   ImGui::Text("No scripts loaded");
    //   return false;
    // }

    // for (uint32_t i = 0; i < loaded_script_objs.size(); ++i) {
    //   const auto& tag = loaded_script_objs[i];
    //   if (!script.IsEmpty() && tag.object_id == script.ScriptHandle()) {
    //     script.selected_script = i;
    //   }
    // }

    // uint32_t slctn = script.selected_script.value_or(0);
    // OE_ASSERT(slctn < loaded_script_objs.size(), "Selected script index out of bounds");

    // const char* current = loaded_script_objs[slctn].name.c_str();
    // ui::ShiftCursor(10.f, 9.f);
    // ImGui::Text("Attach Script");

    // bool editing = script.IsEmpty() || (script.selected_script.has_value() && script.selected_script.value() != slctn);
    // const std::string id = "##attach--script";

    // ImGui::NextColumn();
    // ui::ShiftCursorY(4.f);
    // ImGui::PushItemWidth(-1);
    // if (ui::BeginCombo(id.c_str(), current)) {
    //   for (uint32_t i = 0; i < loaded_script_objs.size(); ++i) {
    //     // const bool is_selected = i == slctn;
    //     // if (ImGui::Selectable(loaded_script_objs[i].name.empty() ? ("EmptyScript" + std::to_string(i)).c_str() : loaded_script_objs[i].name.c_str(), is_selected)) {
    //     //   current = loaded_script_objs[i].name.c_str();
    //     //   script.selected_script = i;
    //     //   editing = editing || true;
    //     // }

    //     // if (is_selected) {
    //     //   ImGui::SetItemDefaultFocus();
    //     // }
    //   }

    //   ui::EndCombo();
    // }
    // ImGui::PopItemWidth();
    // ImGui::NextColumn();
    // ui::Underline();

    // if (editing && ImGui::Button("Attach")) {
    //   auto id = loaded_script_objs[slctn].object_id;
    //   auto name = loaded_script_objs[slctn].name;
    //   auto mod_name = loaded_script_objs[slctn].mod_name;

    //   Opt<std::string> nspace = loaded_script_objs[slctn].nspace.empty() ?
    //     Opt<std::string>{ std::nullopt } :
    //     Opt<std::string>{ loaded_script_objs[slctn].nspace };

    //   script.RemoveScript();
    //   script.AddScript(name, nspace.value_or(""), mod_name);
    // }

    // if (editing && ImGui::Button("Confirm")) {
    //   auto id = loaded_script_objs[slctn].object_id;
    //   auto name = loaded_script_objs[slctn].name;
    //   auto mod_name = loaded_script_objs[slctn].mod_name;

    //   Opt<std::string> nspace = loaded_script_objs[slctn].nspace.empty() ?
    //     Opt<std::string>{ std::nullopt } :
    //     Opt<std::string>{ loaded_script_objs[slctn].nspace };

    //   script.RemoveScript();
    //   script.AddScript(name, nspace.value_or(""), mod_name);
    // }

    auto& script = ent->GetComponent<Script>();
    script.selected_script = std::nullopt;

    if (script.IsEmpty()) {
      ImGui::Text("No script attached");
      /// list scripts out here and attach if selected
      return false;
    }

    std::string name = script.Name();
    std::string lang = script.LanguageType() == LanguageModuleType::CS_MODULE ? "C#" : "Lua";
    bool corrupt = script.IsCorrupt();

    if (corrupt) {
      ImGui::Text("%s corrupt, recompile or reload", name.data());

      if (ImGui::Button("Rebuild Scripts")) {
        // EventQueue::PushEvent<ScriptReloadEvent>();
      }
      return false;
    }

    ui::BeginPropertyGrid();

    ui::ShiftCursor(10.f, 9.f);
    ImGui::Text("%s [ %s ]", name.c_str(), lang.c_str());

    ImGui::NextColumn();
    ui::ShiftCursorY(4.f);
    ImGui::PushItemWidth(-1);

    ImVec2 og_button_txt_align = ImGui::GetStyle().ButtonTextAlign;
    {
      ImGui::GetStyle().ButtonTextAlign = { 0.f, 0.5f };
      float width = ImGui::GetContentRegionAvail().x;
      float item_height = 28.f;

      std::string button_txt = "None";
      /// get full name
      /// button_txt = full_name

      if ((GImGui->CurrentItemFlags & ImGuiItemFlags_MixedValue) != 0) {
        button_txt = "---";
      }

      {
        ScopedColor bg_col(ImGuiCol_WindowBg, ui::theme::property_field);
        ScopedColor button_label_col(ImGuiCol_Text, ui::theme::text);
        ImGui::Button(button_txt.c_str(), { width, item_height });

        const bool hovered = ImGui::IsItemHovered();

        if (hovered) {
          if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            /// open script in text editor
          } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            ImGui::OpenPopup(("##script_popup" + name).c_str());
          }
        }
      }
    }

    ImGui::GetStyle().ButtonTextAlign = og_button_txt_align;
    bool clear = false;
    if (ImGui::BeginPopup(("##script_popup" + name).c_str())) {
      if (clear) {
      }

      ImGui::EndPopup();
    }

    if (ImGui::Button("Remove Script")) {
      script.RemoveScript();
      script.selected_script = std::nullopt;
    }

    ImGui::PopItemWidth();

    ui::EndPropertyGrid();
    ui::Underline();
    ui::BeginPropertyGrid();

    /// display exported properties
    for (auto& [field_id, val] : script.GetFields()) {
      ImGui::PushID(("##script-field" + val.name).c_str());
      ImGui::SetCursorPosX(50.f);

      // if (val.value.IsArray()) {
      //   DrawScriptArray(&val, s);
      // } else {
      // }
      DrawScriptField(&val, script);

      ImGui::PopID();
    }

    ui::EndPropertyGrid();

    return false;
  }

  bool DrawMesh(Entity* ent) {
    auto& mesh = ent->GetComponent<Mesh>();
    AssetHandle original_handle = mesh.handle;

    bool change = false;

    std::set<AssetKey> model_sources = AppState::Assets()->GetAllKeysOfType(AssetType::MODEL_SOURCE);
    if (ui::BeginTreeNode("Model Sources", false)) {
      for (const AssetKey& model : model_sources) {
        AssetMetadata meta = AppState::Assets()->GetMetadata(model);
        if (ImGui::Selectable(meta.path.filename().string().c_str(), mesh.handle == meta.handle)) {
          Ref<ModelSource> source = AssetManager::GetAsset<ModelSource>(model);
          if (source == nullptr) {
            OE_ERROR("Failed to retrieve Model Source [{}] from asset handler", model.file_handle);
          } else {
            Ref<Model> m = ModelSource::CreateModel(source, {});
            mesh.handle = m->handle;
            change = true;
          }
        }
      }

      ui::EndTreeNode();
    }

    if (mesh.handle == 0) {
      ScopedColor red_text(ImGuiCol_Text, ui::theme::muted);
      ImGui::Text("No mesh attached");
      return false;
    }

    bool valid = AppState::Assets()->IsHandleValid(mesh.handle);
    if (!valid) {
      ScopedColor red_text(ImGuiCol_Text, ui::theme::red);
      ImGui::Text("Mesh handle [%lld] invalid", mesh.handle.Get());
      return false;
    }

    if (mesh.handle == original_handle) {
      ScopedColor green_text(ImGuiCol_Text, ui::theme::green);
      ImGui::Text("Mesh handle [%lld] valid", mesh.handle.Get());
      return false;
    }

    Ref<Model> model = AppState::Assets()->GetAsset(mesh.handle);
    if (model == nullptr) {
      ScopedColor red_text(ImGuiCol_Text, ui::theme::red);
      ImGui::Text("Unknown error retrievieng Mesh [{}] from asset handler");
      return false;
    }

    ImGui::Text("Rendering Mesh");

    return mesh.handle != original_handle;
  }

  bool DrawStaticMesh(Entity* ent) {
    auto& mesh = ent->GetComponent<StaticMesh>();
    auto& transform = ent->GetComponent<Transform>();
    AssetHandle original_handle = mesh.handle;

    const char* options[] = { "Empty", "Triangle", "Rect", "Cube", "Sphere", "Capsule" };

    if (ui::PropertyDropdown("Primitive Meshes", options, kCapsuleIdx + 1u, mesh.primitive_selection)) {}

    bool change = false;
    if (mesh.primitive_id != mesh.primitive_selection && ImGui::Button("Confirm Change")) {
      switch (mesh.primitive_selection) {
        case kTriangleIdx: {
          mesh.handle = ModelFactory::CreateTriangle();
          if (!AppState::Assets()->IsValid(mesh.handle)) {
            OE_ERROR("Failed to Create Rect Static Mesh");
          } else {
            change = true;
          }
        } break;

        case kRectIdx: {
          mesh.handle = ModelFactory::CreateRect();
          if (!AppState::Assets()->IsValid(mesh.handle)) {
            OE_ERROR("Failed to Create Rect Static Mesh");
          } else {
            change = true;
          }
        } break;

        case kCubeIdx: {
          mesh.handle = ModelFactory::CreateBox();
          if (!AppState::Assets()->IsValid(mesh.handle)) {
            OE_ERROR("Failed to Create Cube Static Mesh");
          } else {
            change = true;
          }
        } break;

        case kSphereIdx: {
          mesh.handle = ModelFactory::CreateSphere(transform.scale.x / 2.f);
          if (!AppState::Assets()->IsValid(mesh.handle)) {
            OE_ERROR("Failed to Create Sphere Static Mesh");
          } else {
            change = true;
          }
        } break;

        case kCapsuleIdx: {
          mesh.handle = ModelFactory::CreateCapsule(transform.scale.x / 2.f, transform.scale.y);
          if (!AppState::Assets()->IsValid(mesh.handle)) {
            OE_ERROR("Failed to Create Capsule Static Mesh");
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
      mesh.handle = 0;
      return false;
    }

    if (mesh.handle == 0) {
      ScopedColor red_text(ImGuiCol_Text, ui::theme::muted);
      ImGui::Text("No mesh attached");
      return false;
    }

    bool valid = AppState::Assets()->IsHandleValid(mesh.handle);
    if (!valid) {
      ScopedColor red_text(ImGuiCol_Text, ui::theme::red);
      ImGui::Text("Mesh handle [%lld] invalid", mesh.handle.Get());
      return false;
    }

    {
      ScopedColor green_text(ImGuiCol_Text, ui::theme::green);
      ImGui::Text("Mesh handle [%lld] valid", mesh.handle.Get());
    }

    Ref<StaticModel> model = AppState::Assets()->GetAsset(mesh.handle);
    if (model == nullptr) {
      ScopedColor red_text(ImGuiCol_Text, ui::theme::red);
      ImGui::Text("Unknown error retrievieng Static Mesh [{}] from asset handler");
      return false;
    }

    ImGui::Text("Rendering Static Mesh");
    /// render mesh

    return mesh.handle != original_handle;
  }

  bool DrawCamera(Entity* ent) {
    auto& camera = ent->GetComponent<Camera>();

    ui::BeginPropertyGrid();

    const char* proj_types[] = {
      "Perspective",
      "Orthographic"
    };

    if (ImGui::Checkbox("Primary Camera", &camera.is_primary)) {
    }

    uint32_t current_proj = camera.camera->GetCameraProjectionType();

    if (ui::PropertyDropdown("Projection", proj_types, 2, current_proj)) {
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
    ImGui::Checkbox("Pinned to Object Position", &camera.pinned_to_entity_position);

    ImGui::NextColumn();

    if (!camera.pinned_to_entity_position) {
      ui::widgets::DrawVec3Control("Position", camera.camera->position, position_modified);
      ui::widgets::DrawVec3Control("Orientation", camera.camera->euler_angles, orientation_modified);
    }

    ImGui::NextColumn();

    switch (camera.camera->GetCameraProjectionType()) {
      case CameraProjectionType::PERSPECTIVE: {
        ui::DrawTodoReminder("PERSPECTIVE CAMERA OPTIONS");
      } break;

      case CameraProjectionType::ORTHOGRAPHIC: {
        ui::DrawTodoReminder("ORTHOGRAPHIC CAMERA OPTIONS");
      } break;

      default:
        ScopedColor col(ImGuiCol_Text, ui::theme::red);
        ImGui::Text("Invalid Camera Projection Type! Camera Corrupt");
        break;
    }

    ui::EndPropertyGrid();

    return false;
  }

  bool DrawRigidBody2D(Entity* ent) {
    ui::BeginPropertyGrid();

    RigidBody2D& body = ent->GetComponent<RigidBody2D>();

    const char* body_type_strings[] = {
      "Static", "Kinematic", "Dynamic"
    };

    uint32_t selected = body.type;
    if (selected >= INVALID_PHYSICS_BODY) {
      ScopedColor red(ImGuiCol_Text, ui::theme::red);
      ImGui::Text("Invalid valid for Rigid Body 2D body type : %d", body.type);
    } else {
      if (ui::PropertyDropdown("Type", body_type_strings, 3, selected)) {
        body.type = static_cast<PhysicsBodyType>(selected);
        ent->UpdateComponent<RigidBody2D>(body);
      }

      if (body.type == PhysicsBodyType::DYNAMIC) {
        ui::BeginPropertyGrid();

        ui::Property("Mass", &body.mass);
        ui::Property("Linear Drag", &body.linear_drag);
        ui::Property("Angular Drag", &body.angular_drag);
        ui::Property("Gravity Scale", &body.gravity_scale);
        ui::Property("Fixed Rotation", &body.fixed_rotation);
        ui::Property("Bullet", &body.bullet);

        ui::EndPropertyGrid();
      }
    }

    ui::EndPropertyGrid();

    return false;
  }

  bool DrawRigidBody(Entity* ent) {
    ui::BeginPropertyGrid();

    RigidBody& body = ent->GetComponent<RigidBody>();

    const char* body_type_strings[] = {
      "Static", "Kinematic", "Dynamic"
    };

    const char* collision_detection_type_strings[] = {
      "Discrete", "Continuous"
    };

    uint32_t selected = body.type;
    if (selected >= INVALID_PHYSICS_BODY) {
      ScopedColor red(ImGuiCol_Text, ui::theme::red);
      ImGui::Text("Invalid valid for Rigid Body 2D body type : %d", body.type);
    } else {
      if (ui::PropertyDropdown("Type", body_type_strings, 3, selected)) {
        body.type = static_cast<PhysicsBodyType>(selected);
        body.physics_body->SetType(body.type);
      }

      if (body.type == PhysicsBodyType::DYNAMIC) {
        ui::BeginPropertyGrid();

        ui::Property("Mass", &body.mass);
        ui::Property("Linear Drag", &body.linear_drag);
        ui::Property("Angular Drag", &body.angular_drag);

        ui::Property("Gravity Disabled", &body.disable_gravity);
        ui::Property("Is Trigger", &body.is_trigger);

        selected = body.collision_type;
        if (ui::PropertyDropdown("Collision Detection", collision_detection_type_strings, 2, selected)) {
          body.collision_type = static_cast<CollisionDetectionType>(selected);
          ent->UpdateComponent<RigidBody>(body);
        }

        ui::Property("Max Linear Velocity", &body.max_linear_velocity);
        ui::Property("Max Angular Velocity", &body.max_angular_velocity);

        ui::EndPropertyGrid();
      }
    }

    ui::EndPropertyGrid();

    return false;
  }

  bool DrawCollider2D(Entity* ent) {
    Collider2D& collider = ent->GetComponent<Collider2D>();

    ui::BeginPropertyGrid();

    static const char* collider_type_strings[] = {
      "Box", "Circle", "Polygon"
    };

    ScopedColor red(ImGuiCol_Text, ui::theme::red);
    ImGui::Text("Collider 2D ui not implemented yet");

    ui::EndPropertyGrid();

    return false;
  }

  bool DrawCollider(Entity* ent) {
    auto& collider = ent->GetComponent<Collider>();

    const Transform& transform = ent->GetComponent<Transform>();

    static const char* collider_type_strings[] = { "Box", "Sphere", "Capsule", "Convex Mesh", "Concave Mesh", "Compound Shape" };

    ui::BeginPropertyGrid();
    if (collider.shape == nullptr) {
      ScopedColor red(ImGuiCol_Text, ui::theme::red);
      ImGui::Text("Collider shape is null");
      ui::EndPropertyGrid();
      return false;
    }

    bool changed = false;
    if (ui::PropertyDropdown("Collider Type", collider_type_strings, PhysicsShape::Shape::CONCAVE_MESH, collider.shape_idx)) {}

    if (collider.shape_idx != collider.shape->ShapeType() && ImGui::Button("Confirm Change")) {
      changed = true;
      SceneMetadata* md = AppState::Scenes()->ActiveScene();
      OE_ASSERT(md != nullptr, "No active scene found!");
      OE_ASSERT(md->scene != nullptr, "No active scene found!");
      Ref<Scene> scene = md->scene;
      Ref<PhysicsWorld> world = scene->GetPhysicsWorld();
      OE_ASSERT(world != nullptr, "Somehow added a rigid body component without active 3D physics");

      uint32_t idx = collider.shape_idx;
      Ref<PhysicsShape> shape = nullptr;

      switch (idx) {
        case PhysicsShape::Shape::BOX: {
          glm::vec3 half_extents = transform.scale / 2.f;
          shape = world->CreateBoxShape(half_extents);
        } break;

        case PhysicsShape::Shape::SPHERE: {
          shape = world->CreateSphereShape(transform.scale.x / 2.f);
        } break;

        case PhysicsShape::Shape::CAPSULE: {
          shape = world->CreateCapsuleShape(transform.scale.x / 2.f, transform.scale.y);
        } break;

        case PhysicsShape::Shape::CONVEX_MESH: {
          if (!ent->HasAnyComponent<StaticMesh, Mesh>()) {
            OE_ERROR("Can not create physics mesh without mesh!");
            return false;
          }

          if (ent->HasComponent<StaticMesh>()) {
            auto& mesh = ent->GetComponent<StaticMesh>();
            Ref<Model> model = AssetManager::GetAsset<StaticModel>(mesh.handle);
            if (model == nullptr) {
              OE_ERROR("Model asset is null");
              return false;
            }

            Ref<ModelSource> source = model->GetModelSource();
            if (source == nullptr) {
              OE_ERROR("Model source is null");
              return false;
            }

            const std::vector<Vertex>& vertices = source->Vertices();
            const std::vector<Index>& idxs = source->Indices();
            uint32_t num_faces = idxs.size();
            shape = world->CreateConvexMeshShape(vertices, idxs, num_faces);
          } else if (ent->HasComponent<Mesh>()) {
            auto& mesh = ent->GetComponent<Mesh>();
            Ref<Model> model = AssetManager::GetAsset<Model>(mesh.handle);
            if (model == nullptr) {
              OE_ERROR("Model asset is null");
              return false;
            }

            Ref<ModelSource> source = model->GetModelSource();
            if (source == nullptr) {
              OE_ERROR("Model source is null");
              return false;
            }

            const std::vector<SubMesh>& submeshes = source->SubMeshes();
            const std::vector<uint32_t>& submesh_idxs = model->SubMeshes();
            const std::vector<Vertex>& vertices = source->Vertices();
            const std::vector<Index>& idxs = source->Indices();
            uint32_t num_faces = idxs.size();

            // std::vector<Ref<PhysicsShape>> shapes = {};
            // for (const uint32_t sm_idx : submesh_idxs) {
            //   OE_ASSERT(sm_idx < submeshes.size(), "Submesh index out of bounds");
            //   const SubMesh& sm = submeshes[sm_idx];
            //   std::vector<Vertex> verts;
            //   std::vector<Index> indices;

            //   for (uint32_t i = sm.base_vertex; i < sm.base_vertex + sm.vert_cnt; ++i) {
            //     verts.push_back(vertices[i]);
            //   }

            //   for (uint32_t i = sm.base_idx; i < sm.base_idx + sm.idx_cnt; ++i) {
            //     indices.push_back(idxs[i]);
            //   }

            // Ref<PhysicsShape> sub_shape = world->CreateConvexMeshShape(verts, indices, num_faces);
            //   shapes.push_back(sub_shape);
            // }

            // shape = world->CreateCompoundShape(shapes);

            shape = world->CreateConvexMeshShape(vertices, idxs, num_faces);

            ui::EndPropertyGrid();
            return changed;
          } else {
            OE_ASSERT(false, "Entity does not have mesh component");
          }
        } break;
        case PhysicsShape::Shape::CONCAVE_MESH: {
          OE_ERROR("Concave mesh collider not implemented yet");
          return false;
        } break;
        default:
          OE_ERROR("Unimplemented or invalid collider shape type : {}", collider.shape_idx);
          return false;
      }

      if (shape == nullptr) {
        OE_ERROR("Failed to create physics shape");
      } else {
        collider.shape = shape;
      }
    }

    ui::EndPropertyGrid();
    return changed;
  }

  bool DrawPhysicsObject(Entity* ent) {
    bool changed_body = DrawRigidBody(ent);
    bool changed_collider = DrawCollider(ent);

    if (changed_collider) {
      auto& collider = ent->GetComponent<Collider>();
      auto& body = ent->GetComponent<RigidBody>();

      if (body.physics_body != nullptr) {
        body.physics_body->AddCollider(collider.shape);
      }
    }

    return changed_body || changed_collider;
  }

  bool DrawLightSource(Entity* ent) {
    OE_ASSERT(ent != nullptr, "Entity is null");
    OE_ASSERT(ent->HasComponent<LightSource>(), "Entity does not have LightSource component");

    LightSource& light = ent->GetComponent<LightSource>();

    static uint32_t selected = (uint32_t)light.type;
    static int32_t count = 2;
    static const char* light_types[] = {
      "Directional",
      "Point",
    };
    bool changed = ui::PropertyDropdown("Light Source Type", light_types, count, selected);
    if (changed) {
      light.type = static_cast<LightSourceType>(selected);
    }

    switch (light.type) {
      case DIRECTION_LIGHT_SRC: {
        bool light_direction_modified = false;
        glm::vec3 direction = light.direction_light.direction;
        if (ui::widgets::DrawVec3Control("Direction", direction, light_direction_modified, 0.f,  /// replace this value from redo/undo stack
                                         100.f, ui::VectorAxis::ZERO, glm::zero<glm::vec3>(), glm::zero<glm::vec3>(), 0.1f)) {
          light.direction_light.direction = glm::vec4(direction, 1.f);
        }

        ImGui::Text("Color");
        ImGui::ColorEdit3("##Direction Light Color", glm::value_ptr(light.direction_light.color), ImGuiColorEditFlags_NoInputs);
      } break;

      case POINT_LIGHT_SRC: {
        bool light_position_modified = false;
        glm::vec3 position = light.pointlight.position;
        if (ui::widgets::DrawVec3Control("Position", position, light_position_modified, 0.f,  /// replace this value from redo/undo stack
                                         100.f, ui::VectorAxis::ZERO, glm::zero<glm::vec3>(), glm::zero<glm::vec3>(), 0.1f)) {
          light.pointlight.position = glm::vec4(position, 1.f);
        }

        ImGui::Text("Color");
        ImGui::ColorEdit3("##Point Light Color", glm::value_ptr(light.pointlight.color), ImGuiColorEditFlags_NoInputs);

        if (ui::Property("Radius", &light.pointlight.radius, 0.f, 1000.f)) {}
        if (ui::Property("Constant", &light.pointlight.constant, 0.f, 1000.f)) {}
        if (ui::Property("Linear", &light.pointlight.linear, 0.f, 1000.f)) {}
        if (ui::Property("Quadratic", &light.pointlight.quadratic, 0.f, 1000.f)) {}
      } break;

      default:
        break;
    }
    return false;
  }

}  // namespace other
