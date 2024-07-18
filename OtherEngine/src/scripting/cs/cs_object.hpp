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
      CsObject(const std::string& module_name , const std::string& name , CsTypeData* type_data , MonoObject* instance , MonoImage* asm_image ,  
                MonoDomain* app_domain , UUID class_id , CsCache* script_cache , Opt<std::string> name_space = std::nullopt) 
        : ScriptObject(LanguageModuleType::CS_MODULE , module_name , name , name_space) , class_id(class_id) , 
          type_data(type_data) , instance(instance) , app_domain(app_domain) , asm_image(asm_image) , 
          script_cache(script_cache) {}
      virtual ~CsObject() override {}

      CsCache* GetScriptCache() const;

      virtual void InitializeScriptMethods() override;
      virtual void InitializeScriptFields() override;
      virtual void UpdateNativeFields() override;
       
      virtual Opt<Value> GetField(const std::string& name) override;
      virtual void SetField(const std::string& name , const Value& value) override;

      virtual void OnBehaviorLoad() override;
      virtual void Initialize() override;
      virtual void Shutdown() override;
      virtual void OnBehaviorUnload() override;

      virtual void Start() override;
      virtual void Stop() override; 

      virtual void EarlyUpdate(float dt) override;
      virtual void Update(float dt) override;
      virtual void LateUpdate(float dt) override;

      virtual void Render() override;
      virtual void RenderUI() override;

    private:
      /// class data
      UUID class_id = 0;
      CsTypeData* type_data = nullptr;
      MonoObject* instance = nullptr;
      MonoVTable* obj_vtable = nullptr;

      /// dll data
      MonoDomain* app_domain = nullptr;
      MonoImage* asm_image = nullptr;
      CsCache* script_cache = nullptr;

      std::map<UUID , std::vector<MonoObject*>> attribute_instances;

      MonoMethod* on_load_method = nullptr;
      MonoMethod* initialize_method = nullptr;
      MonoMethod* shutdown_method = nullptr;
      MonoMethod* on_unload_method = nullptr;

      MonoMethod* on_start = nullptr;
      MonoMethod* on_stop = nullptr;

      MonoMethod* early_update_method = nullptr;
      MonoMethod* update_method = nullptr;
      MonoMethod* late_update_method = nullptr;

      MonoMethod* render_method = nullptr;
      MonoMethod* render_ui_method = nullptr;

      Opt<Value> GetMonoField(MonoClassField* field , bool is_array = false);
      Opt<Value> GetMonoProperty(MonoMethod* getter , bool is_array = false);

      void SetMonoField(MonoClassField* field , const Value& value , bool is_array = false);
      void SetMonoProperty(MonoMethod* setter , const Value& value , bool is_array = false);

      Opt<Value> CallMonoMethod(MonoMethod* method , uint32_t argc = 0 , Parameter* args = nullptr);
      
      void OnSetEntityId() override;
      virtual Opt<Value> OnCallMethod(const std::string_view name , std::span<Value> value) override;
      virtual Opt<Value> OnCallMethod(const std::string_view name , Parameter* args , uint32_t argc) override;
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_OBJECT_HPP
