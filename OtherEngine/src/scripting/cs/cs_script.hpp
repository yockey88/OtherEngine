/**
 * \file scripting/cs/cs_script.hpp
 **/
#ifndef OTHER_ENGINE_CS_SCRIPT_HPP
#define OTHER_ENGINE_CS_SCRIPT_HPP

#include <map>

#include <Native/hosting/assembly.hpp>

#include "core/UUID.hpp"
#include "scripting/script_module.hpp"
#include "scripting/cs/cs_object.hpp"

namespace other {

  class CsScript : public ScriptModule {
    public:
      CsScript(const std::string& module_name)
        : ScriptModule(LanguageModuleType::CS_MODULE , module_name) {}

      virtual void Initialize() override;
      virtual void Shutdown() override;
      virtual void Reload() override;
      virtual bool HasScript(UUID id) const override;
      virtual bool HasScript(const std::string_view name , const std::string_view nspace = "") const override;
      virtual ScriptObject* GetScript(const std::string& name , const std::string& nspace = "") override;
      
      virtual std::vector<ScriptObjectTag> GetObjectTags() override;

    private:
  };

} // namespace other

#endif // !OTHER_ENGINE_MONO_SCRIPT_MODULE_HPP
