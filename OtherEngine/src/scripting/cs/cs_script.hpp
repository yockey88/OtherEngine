/**
 * \file scripting/cs/cs_script.hpp
 **/
#ifndef OTHER_ENGINE_CS_SCRIPT_HPP
#define OTHER_ENGINE_CS_SCRIPT_HPP

#include <Native/hosting/type.hpp>
#include <Native/hosting/assembly.hpp>

#include "core/UUID.hpp"
#include "scripting/script_module.hpp"
#include "scripting/script_object.hpp"
#include "scripting/cs/cs_object.hpp"

using dotother::ref;
using dotother::Assembly;
using dotother::Type;

namespace other {

  class CsScript : public ScriptModule {
    public:
      CsScript(const std::string& module_name , ref<Assembly> assembly)
        : ScriptModule(LanguageModuleType::CS_MODULE , module_name), assembly(assembly) {}

      virtual void Initialize() override;
      virtual void Shutdown() override;
      virtual void Reload() override;
      virtual bool HasScript(UUID id) const override;
      virtual bool HasScript(const std::string_view name , const std::string_view nspace = "") const override;
      virtual Ref<ScriptObject> GetScriptObject(const std::string& name , const std::string& nspace) override;
      
      virtual std::vector<ScriptObjectTag> GetObjectTags() override;

    private:
      ref<Assembly> assembly = nullptr;
      std::map<UUID , Ref<ScriptObjectHandle<CsObject>>> loaded_objects;
  };

} // namespace other

#endif // !OTHER_ENGINE_CS_SCRIPT_HPP
