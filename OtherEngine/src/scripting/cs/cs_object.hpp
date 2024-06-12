/**
 * \file scripting/cs/cs_object.hpp
*/
#ifndef OTHER_ENGINE_CS_OBJECT_HPP
#define OTHER_ENGINE_CS_OBJECT_HPP

#include <map>

#include <mono/metadata/object.h>

#include "core/UUID.hpp"
#include "scripting/script_object.hpp"

namespace other {

  class CsObject : public ScriptObject {
    public:
      CsObject() 
        : ScriptObject("[Empty Script Object]" , "C#") {}
      CsObject(const std::string& name , MonoClass* klass , MonoObject* instance , MonoImage* asm_image , UUID asm_id ,
                MonoDomain* app_domain) 
        : ScriptObject(name , "C#") , asm_id(asm_id) , app_domain(app_domain) , asm_image(asm_image) , klass(klass) , instance(instance) {}
      virtual ~CsObject() override {}

      virtual void InitializeScriptMethods() override;
      virtual void InitializeScriptFields() override;
      virtual Opt<Value> CallMethod(const std::string& name , Parameter* args , uint32_t argc) override;

      virtual void Initialize() override;
      virtual void Update(float dt) override;
      virtual void Render() override;
      virtual void RenderUI() override;
      virtual void Shutdown() override;

    private:
      UUID asm_id = 0;

      MonoDomain* app_domain = nullptr;
      MonoImage* asm_image = nullptr;
      MonoClass* klass = nullptr;
      MonoObject* instance = nullptr;

      MonoMethod* initialize_method = nullptr;
      MonoMethod* update_method = nullptr;
      MonoMethod* render_method = nullptr;
      MonoMethod* render_ui_method = nullptr;
      MonoMethod* shutdown_method = nullptr;
      std::map<UUID , MonoMethod*> other_methods;

      friend class MonoScriptModule;

      Opt<Value> CallMonoMethod(MonoMethod* method , uint32_t argc = 0 , Parameter* args = nullptr);
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_OBJECT_HPP
