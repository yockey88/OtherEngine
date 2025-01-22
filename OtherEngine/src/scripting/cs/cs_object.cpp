/**
 * \file scripting/cs/cs_object.cpp
 */
#include "scripting/cs/cs_object.hpp"

#include <hosting/assembly.hpp>
#include <hosting/field.hpp>

#include "core/defines.hpp"
#include "core/logger.hpp"

#include "scripting/cs/cs_module.hpp"
#include "scripting/script_field.hpp"
#include "scripting/script_method.hpp"

using dotother::Type;

namespace other {

  namespace {

    Value GetValueFromField(HostedObject& obj, const std::string_view name, ValueType type) {
      switch (type) {
        case ValueType::BOOL:
          return obj.GetField<bool>(name);
        case ValueType::CHAR:
          return obj.GetField<char>(name);

        case ValueType::INT8:
          return obj.GetField<int8_t>(name);
        case ValueType::INT16:
          return obj.GetField<int16_t>(name);
        case ValueType::INT32:
          return obj.GetField<int32_t>(name);
        case ValueType::INT64:
          return obj.GetField<int64_t>(name);

        case ValueType::UINT8:
          return obj.GetField<uint8_t>(name);
        case ValueType::UINT16:
          return obj.GetField<uint16_t>(name);

        case ValueType::SAMPLER2D:
        case ValueType::SAMPLER2D_ARRAY:
        case ValueType::UINT32:
          return obj.GetField<uint32_t>(name);
        case ValueType::UINT64:
          return obj.GetField<uint64_t>(name);

        case ValueType::FLOAT:
          return obj.GetField<float>(name);
        case ValueType::DOUBLE:
          return obj.GetField<double>(name);

        case ValueType::VEC2:
          return obj.GetField<glm::vec2>(name);
        case ValueType::VEC3:
          return obj.GetField<glm::vec3>(name);
        case ValueType::VEC4:
          return obj.GetField<glm::vec4>(name);
        case ValueType::MAT2:
          return obj.GetField<glm::mat2>(name);
        case ValueType::MAT3:
          return obj.GetField<glm::mat3>(name);
        case ValueType::MAT4:
          return obj.GetField<glm::mat4>(name);

        case ValueType::STRING:
          return obj.GetField<std::string>(name);

        case ValueType::ENTITY:
        case ValueType::ASSET:
          return obj.GetField<uint64_t>(name);

        case ValueType::USER_TYPE:
        case ValueType::OPAQUE_HANDLE:
        case ValueType::EMPTY_TYPE:
          OE_ERROR("Unimplemented value type");
          return Value{};

        default:
          OE_ASSERT(false, "Invalid value type");
      }
      OE_ASSERT(false, "Invalid value type");
    }

  }  // anonymous namespace

  CsObject::CsObject(CsModule* host, ScriptModule* module, ref<Assembly>& assembly, Type& type, const std::string_view name, std::string_view nspace, UUID handle)
      : ScriptObjectHandle(LanguageModuleType::CS_MODULE, module, handle, "[Empty Script Object]"),
        host(host), assembly(assembly), type(type) {
    OE_ASSERT(host != nullptr, "Host is null");
    OE_ASSERT(module != nullptr, "Module is null");
    OE_ASSERT(assembly != nullptr, "Assembly is null");
    OE_ASSERT(type.handle != -1, "Type is null");
    hosted_object = type.NewInstance();
    script_name = std::string{ name };
    script_instance_name = script_name;
    name_space = nspace.empty() ? std::nullopt : std::make_optional(std::string{ nspace });
  }

  void CsObject::InitializeScriptMethods() {
    const std::vector<dotother::Method>& ms = type.Methods();
    for (const auto& method : ms) {
      ScriptMethod sm;

      std::string name = method.GetName();

      sm.id = FNV(name);
      sm.name = name;

      methods[sm.id] = sm;
    }
  }

  void CsObject::InitializeScriptFields() {
    const auto& type_map = host->GetTypeMap();

    std::vector<dotother::Field>& fs = type.Fields();
    for (auto& field : fs) {
      std::string name = field.GetName();
      ScriptField sf = fields[FNV(name)];
      sf.id = FNV(name);
      sf.name = name;

      /// TODO: get attributes from field
      sf.bounds = std::nullopt;

      // ValueType type = ValueType::EMPTY_TYPE;
      // {
      //   dotother::Type& t = field.GetType();
      //   // auto itr = type_map.find(t.handle);
      //   // if (itr != type_map.end()) {
      //   //   OE_INFO("Found type {} for field {}", t.FullName(), name);
      //   // }
      // }

      // sf.value = GetValueFromField(hosted_object, name, type);
    }
  }

  void CsObject::UpdateNativeFields() {
  }

}  // namespace other
