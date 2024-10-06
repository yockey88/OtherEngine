/**
 * \file scripting/script_object.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_OBJECT_HPP
#define OTHER_ENGINE_SCRIPT_OBJECT_HPP

#include <string>
#include <map>
#include <type_traits>

#include <entt/entity/fwd.hpp>
#include <hosting/native_string.hpp>

#include "core/ref_counted.hpp"
#include "core/ref.hpp"

#include "scripting/script_defines.hpp"
#include "scripting/script_field.hpp"

namespace other {

  class ScriptModule;

  /// TODO: fix how scripts are loaded so this can be an asset and retrieved
  ///       from the asset manager
  class ScriptObject : public RefCounted /* : public Asset */ {
    public:
      /// OE_ASSET(SCRIPT);

      ScriptObject(LanguageModuleType lang_type , ScriptModule* module , UUID script_handle , const std::string& instance_name) 
        : lang_type(lang_type) , module(module) , script_handle(script_handle) , script_instance_name(instance_name) {}
      virtual ~ScriptObject() {}

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
      
      struct Handles {
        UUID entity_id;
        entt::entity entity_handle;
        void* native_object_handle;
      } handles;

      bool is_initialized = false;
      bool is_corrupt = false;

      std::string script_instance_name;
      Opt<std::string> name_space;
      std::string script_name;

      std::map<UUID , ScriptField> fields;
  };

  template <typename SO>
  concept script_object_t = std::is_base_of_v<ScriptObject, SO>;

  class Entity;

  template <typename SO>
  class ScriptObjectHandle : public ScriptObject {
    public:
      ScriptObjectHandle(LanguageModuleType lang_type , ScriptModule* module , UUID script_handle , const std::string& instance_name)
        : ScriptObject(lang_type , module, script_handle , instance_name) {}
      virtual ~ScriptObjectHandle() {}

      template <typename R , typename... Args>
      constexpr auto CallMethod(const std::string& method , Args&&... args) -> R {
        return static_cast<SO*>(this)->template CallMethod<R , Args...>(method , std::forward<Args>(args)...);
      }

      template <typename T>
      constexpr auto SetField(const std::string& name , T&& arg) -> void {
        static_cast<SO*>(this)->SetField(name , std::forward<T>(arg));
      }

      template <typename R>
      constexpr auto GetField(const std::string& name) -> R {
        return static_cast<SO*>(this)->template GetField<R>(name);
      }

      template <typename T>
      constexpr auto SetProperty(const std::string_view name , T&& arg) -> void {
        static_cast<SO*>(this)->template SetProperty<T>(name , std::forward<T>(arg));
      }

      template <typename R>
      constexpr auto GetProperty(const std::string_view name) -> R {
        return static_cast<SO*>(this)->template GetProperty<R>(name);
      }

      void SetHandles(UUID id , entt::entity handle , void* native_handle) {
        handles = {
          .entity_id = id ,
          .entity_handle = handle ,
          .native_object_handle = native_handle ,
        };

        SetProperty("NativeHandle" , native_handle);
        SetProperty("ObjectID" , id.Get());
        SetProperty("EntityID" , (uint32_t)handle);
      }

      void OnBehaviorLoad() override {
        CallMethod<void>("OnBehaviorLoad");
      }
      
      void OnBehaviorUnload() override {
        CallMethod<void>("OnBehaviorUnload");
      }
      
      void Initialize() override {
        CallMethod<void>("OnInitialize");
      }
      
      void Shutdown() override {
        CallMethod<void>("OnShutdown");
      } 

      void Start() override {
        CallMethod<void>("OnStart");
      }
      
      void Stop() override {
        CallMethod<void>("OnStop");
      }

      void EarlyUpdate(float dt) override {
        CallMethod<void, float>("EarlyUpdate" , std::forward<float>(dt));
      }
      
      void Update(float dt) override {
        CallMethod<void, float>("Update" , std::forward<float>(dt));
      }

      void LateUpdate(float dt) override {
        CallMethod<void, float>("LateUpdate" , std::forward<float>(dt));
      }

      void Render() override {
        CallMethod<void>("Render");
      }
      
      void RenderUI() override {
        CallMethod<void>("RenderUI");
      }
  };
      
  template <typename SO>
  using ScriptRef = Ref<ScriptObjectHandle<SO>>;

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_OBJECT_HPP
