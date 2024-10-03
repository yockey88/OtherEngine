/**
 * \file scripting/script_object.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_OBJECT_HPP
#define OTHER_ENGINE_SCRIPT_OBJECT_HPP

#include <concepts>
#include <string>
#include <map>
#include <type_traits>

#include "core/ref_counted.hpp"
#include "core/ref.hpp"
#include "core/value.hpp"

#include "scripting/script_defines.hpp"
#include "scripting/script_field.hpp"

namespace other {

  class ScriptModule;

  template <typename T>
  class ScriptObjectHandle;

  /// TODO: fix how scripts are loaded so this can be an asset and retrieved
  ///       from the asset manager
  class ScriptObject : public RefCounted /* : public Asset */ {
    public:
      /// OE_ASSET(SCRIPT);

      ScriptObject(LanguageModuleType lang_type , ScriptModule* module , UUID script_handle , const std::string& instance_name) 
        : lang_type(lang_type) , module(module) , script_handle(script_handle) , script_instance_name(instance_name) {}
      virtual ~ScriptObject() {}

      template <typename SO , typename R , typename... Args>
      constexpr auto CallMethod(const std::string& method , Args&&... args) -> R;

      void SetEntityId(UUID id);
      UUID GetEntityId() const;

      const std::string_view ScriptInstanceName() const;
      const Opt<std::string> NameSpace() const;
      const std::string_view Name() const;
          
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
      ScriptModule* module = nullptr;

      UUID script_handle;
      UUID entity_id;

      bool is_initialized = false;
      bool is_corrupt = false;

      std::string script_instance_name;
      Opt<std::string> name_space;
      std::string script_name;

      std::map<UUID , ScriptField> fields;
      
      virtual void OnSetEntityId() = 0;
  };

  template <typename SO>
  concept script_object_t = std::is_base_of_v<ScriptObject,SO>;

  template <typename T>
  class ScriptObjectHandle : public ScriptObject {
    public:
      ScriptObjectHandle(LanguageModuleType lang_type , ScriptModule* module , UUID script_handle , const std::string& instance_name)
        : ScriptObject(lang_type , module, script_handle , instance_name) {}
      virtual ~ScriptObjectHandle() {}

      template <typename R , typename... Args>
      constexpr auto CallMethod(const std::string& method , Args&&... args) -> R {
        return static_cast<T*>(this)->template CallMethod<R , Args...>(method , std::forward<Args>(args)...);
      }

      void OnBehaviorLoad() override {
        CallMethod<void>("OnBehaviorLoad");
      }
      
      void Initialize() override {
        CallMethod<void>("Initialize");
      }
      
      void Shutdown() override {
        CallMethod<void>("Shutdown");
      } 
      
      void OnBehaviorUnload() override {
        CallMethod<void>("OnBehaviorUnload");
      }

      void Start() override {
        CallMethod<void>("Start");
      }
      
      void Stop() override {
        CallMethod<void>("Stop");
      }

      void EarlyUpdate(float dt) override {
        CallMethod<void>("EarlyUpdate" , dt);
      }
      
      void Update(float dt) override {
        CallMethod<void>("Update" , dt);
      }

      void LateUpdate(float dt) override {
        CallMethod<void>("LateUpdate" , dt);
      }

      void Render() override {
        CallMethod<void>("Render");
      }
      
      void RenderUI() override {
        CallMethod<void>("RenderUI");
      }
  };
      
  template <typename SO , typename R , typename... Args>
  constexpr auto ScriptObject::CallMethod(const std::string& method , Args&&... args) -> R {
    Ref<ScriptObjectHandle<SO>> obj = GetObjectHandle<SO>();  
    if (obj == nullptr) {
      if constexpr (std::same_as<R , void>) {
        return;
      } else {
        return R{};
      }
    }
    return obj->template CallMethod<R , Args...>(method , std::forward<Args>(args)...);
  }

  template <typename T>
  using ScriptRef = Ref<ScriptObjectHandle<T>>;

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_OBJECT_HPP
