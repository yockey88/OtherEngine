/**
 * \file scripting/cs/cs_object.hpp
*/
#ifndef OTHER_ENGINE_CS_OBJECT_HPP
#define OTHER_ENGINE_CS_OBJECT_HPP

#include <mono/metadata/object.h>

#include "core/UUID.hpp"
#include "scripting/script_object.hpp"
#include "scripting/cs/cs_script_cache.hpp"

namespace other {

  class CsObject : public ScriptObject {
    public:
      CsObject() 
        : ScriptObject(LanguageModuleType::CS_MODULE , "[Empty Script Object]" , "C#") {}
      CsObject(const std::string& name , CsTypeData* type_data , MonoObject* instance , MonoImage* asm_image ,  
                MonoDomain* app_domain , UUID class_id , CsCache* script_cache) 
        : ScriptObject(LanguageModuleType::CS_MODULE , name , "C#") , class_id(class_id) , type_data(type_data) , instance(instance) , 
          app_domain(app_domain) , asm_image(asm_image) , script_cache(script_cache) {}
      virtual ~CsObject() override {}

      virtual void InitializeScriptMethods() override;
      virtual void InitializeScriptFields() override;
      
      virtual Opt<Value> OnCallMethod(const std::string_view name , std::span<Value> value) override;
      virtual Opt<Value> OnCallMethod(const std::string_view name , Parameter* args , uint32_t argc) override;
      
      virtual Opt<Value> GetField(const std::string& name) override;
      virtual void SetField(const std::string& name , const Value& value) override;

      virtual void Initialize() override;
      virtual void Update(float dt) override;
      virtual void Render() override;
      virtual void RenderUI() override;
      virtual void Shutdown() override;

    private:
      /// class data
      UUID class_id = 0;
      CsTypeData* type_data = nullptr;
      MonoObject* instance = nullptr;

      /// dll data
      MonoDomain* app_domain = nullptr;
      MonoImage* asm_image = nullptr;
      CsCache* script_cache = nullptr;

      /// saved for fast access
      MonoMethod* initialize_method = nullptr;
      MonoMethod* update_method = nullptr;
      MonoMethod* render_method = nullptr;
      MonoMethod* render_ui_method = nullptr;
      MonoMethod* shutdown_method = nullptr;

      Opt<Value> GetMonoField(MonoClassField* field);
      Opt<Value> GetMonoProperty(MonoMethod* getter);

      void SetMonoField(MonoClassField* field , const Value& value);
      void SetMonoProperty(MonoMethod* setter , const Value& value);

      Opt<Value> CallMonoMethod(MonoMethod* method , uint32_t argc = 0 , Parameter* args = nullptr);
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_OBJECT_HPP
