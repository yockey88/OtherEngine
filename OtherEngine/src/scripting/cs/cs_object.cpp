/**
 * \file scripting/cs/cs_object.cpp
 */
#include "scripting/cs/cs_object.hpp"

#include <hosting/field.hpp>

#include "core/logger.hpp"

#include "scripting/script_field.hpp"
#include "scripting/script_method.hpp"

using dotother::Type;

namespace other {

  /**
      std::string script_instance_name;
    Opt<std::string> name_space;
    std::string script_name;
     */

  CsObject::CsObject(ScriptModule* module, Type& type, const std::string_view name, std::string_view nspace, UUID handle)
      : ScriptObjectHandle(LanguageModuleType::CS_MODULE, module, handle, "[Empty Script Object]"),
        type(type) {
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
    const std::vector<dotother::Field>& fs = type.Fields();
    for (const auto& field : fs) {
      ScriptField sf;

      std::string name = field.GetName();

      sf.id = FNV(name);
      sf.name = name;

      /// TODO: get attributes from field
      sf.bounds = std::nullopt;

      /// TODO: implement defaults
      // hosted_object.SetField(field.GetName().ToString(), field.GetType().Default());

      fields[sf.id] = sf;
    }
  }

  void CsObject::UpdateNativeFields() {
  }

}  // namespace other
