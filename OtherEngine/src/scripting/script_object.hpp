/**
 * \file scripting/script_object.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_OBJECT_HPP
#define OTHER_ENGINE_SCRIPT_OBJECT_HPP

#include <string>
#include <map>
#include <span>

#include "core/ref_counted.hpp"
#include "core/value.hpp"

#include "scripting/script_defines.hpp"
#include "scripting/script_field.hpp"

namespace other {

  /// TODO: fix how scripts are loaded so this can be an asset and retrieved
  ///       from the asset manager
  class ScriptObject : public RefCounted /* : public Asset */ {
    public:
      /// OE_ASSET(SCRIPT);

      ScriptObject(LanguageModuleType lang_type , const std::string& module_name , 
          const std::string& name , Opt<std::string> name_space = std::nullopt) 
        : lang_type(lang_type) , script_module_name(module_name) , name_space(name_space) , script_name(name) {}
      virtual ~ScriptObject() {}

      template <typename... Args>
      Opt<Value> CallMethod(const std::string_view method , Args&&... args) {
        std::array<Value , sizeof...(Args)> arr = {
          Value{ std::forward<Args>(args) } ...
        };

        return OnCallMethod(method , arr);
      }
      
      template <>
      Opt<Value> CallMethod(const std::string_view method) {
        std::span<Value> empty{};
        return OnCallMethod(method , empty);
      }

      void SetEntityId(UUID id);
      UUID GetEntityId() const;
      
      const std::string& ScriptName() const;
      const Opt<std::string> NameSpace() const;
      const std::string& Name() const;

      LanguageModuleType LanguageType() const;

      void MarkCorrupt();
      bool IsCorrupt() const;
      bool IsInitialized() const;

      std::map<UUID , ScriptField>& GetFields();
      const std::map<UUID , ScriptField>& GetFields() const;

      virtual void InitializeScriptMethods() = 0;
      virtual void InitializeScriptFields() = 0;
      virtual void UpdateNativeFields() = 0;

      virtual Opt<Value> GetField(const std::string& name) = 0;
      virtual void SetField(const std::string& name , const Value& value) = 0;

      virtual void OnBehaviorLoad() = 0;
      virtual void Initialize() = 0;
      virtual void Shutdown() = 0; 
      virtual void OnBehaviorUnload() = 0;

      virtual void Start() = 0;
      virtual void Stop() = 0;

      virtual void EarlyUpdate(float dt) = 0;
      virtual void Update(float dt) = 0;
      virtual void LateUpdate(float dt) = 0;

      virtual void Render() = 0;
      virtual void RenderUI() = 0;
    
    protected:
      LanguageModuleType lang_type;

      UUID entity_id;

      bool is_initialized = false;
      bool is_corrupt = false;

      std::string script_module_name;
      Opt<std::string> name_space;
      std::string script_name;

      std::map<UUID , ScriptField> fields;
      
      virtual void OnSetEntityId() = 0;
      virtual Opt<Value> OnCallMethod(const std::string_view name , std::span<Value> args) = 0;
      virtual Opt<Value> OnCallMethod(const std::string_view name , Parameter* args , uint32_t argc);
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_OBJECT_HPP
