/**
 * \file scripting/cs/cs_script.hpp
 **/
#ifndef OTHER_ENGINE_CS_SCRIPT_HPP
#define OTHER_ENGINE_CS_SCRIPT_HPP

#include <map>

#include <mono/metadata/appdomain.h>

#include "core/UUID.hpp"
#include "scripting/script_module.hpp"
#include "cs_object.hpp"

namespace other {

  class Engine;

  class CsScript : public ScriptModule {
    public:
      CsScript(MonoDomain* root_domain , MonoDomain* app_domain , const std::string& assembly_path)
        : ScriptModule() , root_domain(root_domain) , app_domain(app_domain) , assembly_path(assembly_path) {}

      MonoImage* GetImage() const;
      MonoAssembly* GetAsm() const;

      virtual void Initialize() override;
      virtual void Shutdown() override;
      virtual void Reload() override;
      virtual ScriptObject* GetScript(const std::string& name , const std::string& nspace = "") override;

    private:
      MonoDomain* root_domain = nullptr;
      MonoDomain* app_domain = nullptr;

      MonoImage* assembly_image = nullptr;
      MonoAssembly* assembly = nullptr;

      UUID script_id;
      std::map<UUID , MonoClass*> classes;
      std::map<UUID , CsObject> loaded_objects;
      // garbage_collector stuff

      std::string assembly_path;

      MonoClass* GetClass(const std::string& name , const std::string& nspace);
  };

} // namespace other

#endif // !OTHER_ENGINE_MONO_SCRIPT_MODULE_HPP