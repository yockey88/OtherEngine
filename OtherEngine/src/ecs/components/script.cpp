/**
 * \file ecs/components/script.cpp
 **/
#include "ecs/components/script.hpp"

#include <algorithm>
#include <iterator>

#include "core/config_keys.hpp"
#include "core/logger.hpp"

#include "ecs/entity.hpp"
#include "scene/scene.hpp"

#include "scripting/cs/cs_object.hpp"
#include "scripting/script_engine.hpp"

namespace other {

  UUID Script::AddScript(const std::string_view name, const std::string_view nspace, const std::string_view module) {
    OE_ASSERT(parent_handle != nullptr, "Parent handle is null");

    std::string case_ins_name;
    std::ranges::transform(name, std::back_inserter(case_ins_name), ::toupper);
    UUID id = FNV(case_ins_name);

    ScriptRef<CsObject> inst = ScriptEngine::GetObjectRef<CsObject>(name, nspace, module);
    if (inst == nullptr) {
      OE_ERROR("Failed to get script {} from script module {}", name, module);
      return 0;
    } else {
      OE_DEBUG("Adding script {} [{} {}] to entity {}", name, id, module, parent_handle->Name());
    }

    if (script_object != nullptr) {
      RemoveScript();
    }

    object_data = ScriptObjectData{
      .module = std::string{ module },
      .obj_name = std::string{ name },
    };
    script_object = inst;

    inst->SetHandles(parent_uuid, parent_id, parent_handle);
    if (!ValidateScripts()) {
      OE_ERROR("Script [{}::{} , {}] failed validation on entity {}", name, nspace, module, parent_handle->Name());
    } else {
      OE_DEBUG(" > Script {}::{} [{}] added to entity {}", nspace, name, id, parent_handle->Name());
    }

    return id;
  }

  void Script::RemoveScript() {
    script_object = nullptr;
    object_data = {};
  }

  void Script::ApiCall(const std::string_view name) {
    if (script_object != nullptr) {
      script_object->CallMethod<void>(std::string{ name });
    }
  }

  bool Script::IsEmpty() const {
    return script_object == nullptr;
  }

  bool Script::IsCorrupt() const {
    if (script_object != nullptr) {
      return script_object->IsCorrupt();
    }
    return false;
  }

  UUID Script::ScriptHandle() const {
    if (script_object != nullptr) {
      return script_object->GetScriptHandle();
    }
    return 0;
  }

  std::string Script::Name() const {
    if (script_object != nullptr) {
      return std::string{ script_object->Name() };
    }
    return "<empty>";
  }

  LanguageModuleType Script::LanguageType() const {
    if (script_object != nullptr) {
      return script_object->LanguageType();
    }
    return LanguageModuleType::INVALID_LANGUAGE_MODULE;
  }

  std::map<UUID, ScriptField>& Script::GetFields() {
    if (script_object != nullptr) {
      return script_object->GetFields();
    }
    static std::map<UUID, ScriptField> empty_map;
    return empty_map;
  }

  void Script::SetField(const std::string_view name, Value& value) {
    switch (value.GetType()) {
      case ValueType::BOOL:
        script_object->SetField<bool>(std::string{ name }, std::forward<bool>(value.Get<bool>()));
        return;
      case ValueType::CHAR:
        script_object->SetField<char>(std::string{ name }, std::forward<char>(value.Get<char>()));
        return;
      case ValueType::INT8:
        script_object->SetField<int8_t>(std::string{ name }, std::forward<int8_t>(value.Get<int8_t>()));
        return;
      case ValueType::INT16:
        script_object->SetField<int16_t>(std::string{ name }, std::forward<int16_t>(value.Get<int16_t>()));
        return;
      case ValueType::INT32:
        script_object->SetField<int32_t>(std::string{ name }, std::forward<int32_t>(value.Get<int32_t>()));
        return;
      case ValueType::INT64:
        script_object->SetField<int64_t>(std::string{ name }, std::forward<int64_t>(value.Get<int64_t>()));
        return;
      case ValueType::UINT8:
        script_object->SetField<uint8_t>(std::string{ name }, std::forward<uint8_t>(value.Get<uint8_t>()));
        return;
      case ValueType::UINT16:
        script_object->SetField<uint16_t>(std::string{ name }, std::forward<uint16_t>(value.Get<uint16_t>()));
        return;
      case ValueType::UINT32:
        script_object->SetField<uint32_t>(std::string{ name }, std::forward<uint32_t>(value.Get<uint32_t>()));
        return;
      case ValueType::UINT64:
        script_object->SetField<uint64_t>(std::string{ name }, std::forward<uint64_t>(value.Get<uint64_t>()));
        return;
      case ValueType::FLOAT:
        script_object->SetField<float>(std::string{ name }, std::forward<float>(value.Get<float>()));
        return;
      case ValueType::DOUBLE:
        script_object->SetField<double>(std::string{ name }, std::forward<double>(value.Get<double>()));
        return;
      case ValueType::VEC2:
        script_object->SetField<glm::vec2>(std::string{ name }, std::forward<glm::vec2>(value.Get<glm::vec2>()));
        return;
      case ValueType::VEC3:
        script_object->SetField<glm::vec3>(std::string{ name }, std::forward<glm::vec3>(value.Get<glm::vec3>()));
        return;
      case ValueType::VEC4:
        script_object->SetField<glm::vec4>(std::string{ name }, std::forward<glm::vec4>(value.Get<glm::vec4>()));
        return;
      case ValueType::MAT2:
        script_object->SetField<glm::mat2>(std::string{ name }, std::forward<glm::mat2>(value.Get<glm::mat2>()));
        return;
      case ValueType::MAT3:
        script_object->SetField<glm::mat3>(std::string{ name }, std::forward<glm::mat3>(value.Get<glm::mat3>()));
        return;
      case ValueType::MAT4:
        script_object->SetField<glm::mat4>(std::string{ name }, std::forward<glm::mat4>(value.Get<glm::mat4>()));
        return;
      case ValueType::STRING:
        script_object->SetField<std::string>(std::string{ name }, std::forward<std::string>(value.Get<std::string>()));
        return;
      case ValueType::ENTITY:
      case ValueType::ASSET:
      case ValueType::USER_TYPE:
      case ValueType::OPAQUE_HANDLE:
        OE_ERROR("Unimplemented value type");
        return;
      case ValueType::EMPTY_TYPE:
        return;

      default:
        OE_ASSERT(false, "Invalid value type");
    }
  }

  bool Script::ValidateScripts() {
    OE_ASSERT(script_object != nullptr, "Script object is null");

    bool handle_match = false;
    handle_match = script_object->GetProperty<void*>("NativeHandle") == (void*)parent_handle;

    // bool eid_match = script_object->GetProperty<uint32_t>("EntityID") == (uint32_t)parent_id;
    bool oid_match = script_object->GetProperty<uint64_t>("ObjectID") == parent_uuid.Get();

    if (!handle_match) {
      OE_ERROR("NativeHandle mismatch for script {} [{:p} != {:p}]", script_object->Name(), script_object->GetProperty<void*>("NativeHandle"), (void*)parent_handle);
    }

    // if (!eid_match) {
    //   OE_ERROR("EntityID mismatch for script {} [{} != {}]", script_object->Name(), script_object->GetProperty<uint32_t>("EntityID"), (uint32_t)parent_id);
    // }

    if (!oid_match) {
      OE_ERROR("ObjectID mismatch for script {} [{} != {}]", script_object->Name(), script_object->GetProperty<uint64_t>("ObjectID"), parent_uuid.Get());
    }

    return handle_match && oid_match;  // && eid_match
  }

  void Script::SetHandles() {
    OE_ASSERT(parent_handle != nullptr, "Scene handle is not null");
    script_object->SetHandles(parent_uuid, parent_id, (void*)parent_handle);
  }

  void ScriptSerializer::Serialize(std::ostream& stream, Entity* entity, const Ref<Scene>& scene) const {
    const auto& script = entity->GetComponent<Script>();

    SerializeComponentSection(stream, entity, "script");
    // for (const auto& [id, s] : script.GetScripts()) {
    //   // stream << s->ScriptInstanceName() << " = { \"";
    //   // if (s->NameSpace().has_value()) {
    //   //   stream << s->NameSpace().value() << "::";
    //   // }
    //   // stream << s->Name() << "\" }\n";
    // }
  }

  void ScriptSerializer::Deserialize(Entity* entity, const ConfigTable& scene_table, Ref<Scene>& scene) const {
    OE_ASSERT(entity != nullptr && scene != nullptr, "Attempting to deserialize scripts into null entity or scene");
    auto& script = entity->AddComponent<Script>();
    std::string key_value = GetComponentSectionKey(entity->Name(), std::string{ kScriptValue });

    try {
      std::vector<std::string> mod_s = scene_table.GetKeys(key_value);
      if (mod_s.size() > 1) {
        std::stringstream ss;
        for (uint32_t i = 0; i < mod_s.size(); ++i) {
          ss << mod_s[i];
          if (i < mod_s.size() - 1) {
            ss << ", ";
          }
        }
        OE_ERROR("More than one script found for entity {} [{}]", entity->Name(), ss.str());
        return;
      } else if (mod_s.empty()) {
        OE_WARN("No scripts found for entity {}", entity->Name());
        return;
      }
      std::string& mod = mod_s[0];

      std::string s = scene_table.GetVal<std::string>(key_value, mod, true).value();  // true because keys are case-sensitive
      OE_DEBUG("Attaching {} to {}", s, entity->Name());

      UUID sid = script.AddScript(s, "", mod);
      OE_DEBUG("Attached {} [{}]", s, sid);

      // OE_DEBUG("Attached {} scripts to entity {}", script.GetScripts().size(), entity->Name());
      OE_DEBUG("Attached script to entity {}", entity->Name());
    } catch (const std::bad_optional_access& e) {
      OE_ERROR("Failed to attach component {} : {}", key_value, e.what());
    } catch (const std::exception& e) {
      OE_ERROR("Failed to attach component {} : {}", key_value, e.what());
    } catch (...) {
      OE_ERROR("Unknown error while attaching component : {}", key_value);
    }
  }

}  // namespace other
