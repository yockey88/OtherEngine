/**
 * \file scripting/cs/cs_scene_bindings.cpp
 **/
#include "scripting/cs/cs_component_bindings.hpp"

#include <cstdint>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/fwd.hpp>
#include <unordered_map>
#include <functional>

#include <hosting/native_string.hpp>
#include <hosting/type.hpp>

#include "core/logger.hpp"
#include "core/type_data.hpp"
#include "ecs/entity.hpp"
#include "ecs/component.hpp"
#include "ecs/components/relationship.hpp"

#include "ecs/components/transform.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/light_source.hpp"

#include "scripting/script_engine.hpp"
#include "scripting/cs/cs_register_internal_call.hpp"

namespace other {
namespace {

  std::unordered_map<int32_t , std::function<void(Entity*)>> create_component_funcs;
  std::unordered_map<int32_t , std::function<bool(Entity*)>> has_component_funcs;
  std::unordered_map<int32_t , std::function<void(Entity*)>> remove_component_funcs;

  template <typename T>
    requires ComponentType<T>
  static void RegisterComponent(ref<Assembly> assembly , auto... getters_and_setters) {
    const std::string_view component_type = TypeData<T , true>().Name(); 
    std::string name = fmtstr("Other.{}" , component_type);

    if constexpr (std::same_as<T , Mesh>) {
    }

    Type& type = assembly->GetType(name);
    if (type.handle == -1) {
      OE_ASSERT(false , "NO C# type found for {}" , name);
    }

    OE_DEBUG("Registering C# component : {} [{}]" , name , type.handle);

    create_component_funcs[type.handle] = [](Entity* entity) { 
      OE_ASSERT(entity != nullptr , "CALLING CreateComponent on null entity!");
      entity->AddComponent<T>();
    };
    has_component_funcs[type.handle] = [](Entity* entity) {
      OE_ASSERT(entity != nullptr , "CALLING HasComponent on null entity!");
      return entity->HasComponent<T>();
    };
    remove_component_funcs[type.handle] = [](Entity* entity) {
      OE_ASSERT(entity != nullptr , "CALLING RemoveComponent on null entity!");
      entity->RemoveComponent<T>();
    };
    OE_DEBUG(" > Registered C# component : {} [{}]" , name , type.handle);

    if constexpr (sizeof...(getters_and_setters) > 0) {
      // register getters and setters
    }
  }

  template <typename T>
    requires ComponentType<T>
  static void RegisterComponent(std::function<void(Entity*)>&& add_fn , ref<Assembly> assembly) {
    const std::string& component_type = TypeData<T , true>().Name(); 
    std::string name = fmtstr("Other.{}" , component_type);
    OE_DEBUG(" > Registering C# component : {}" , name);

    if constexpr (std::same_as<T , Mesh>) {
    }

    Type& type = assembly->GetType(name);
    if (type.handle == -1) {
      OE_ASSERT(false , "NO C# type found for {}" , name);
    }

    create_component_funcs[type.handle] = std::move(add_fn);
    has_component_funcs[type.handle] = [](Entity* entity) {
      OE_ASSERT(entity != nullptr , "CALLING HasComponent on null entity!");
      return entity->HasComponent<T>();
    };
    remove_component_funcs[type.handle] = [](Entity* entity) {
      OE_ASSERT(entity != nullptr , "CALLING RemoveComponent on null entity!");
      entity->RemoveComponent<T>();
    };
  }

  bool NativeHasComponent(Entity* entity , int32_t type) {
    OE_ASSERT(entity != nullptr , "Entity is null!");
    auto itr = has_component_funcs.find(type);
    if (itr == has_component_funcs.end()) {
      OE_WARN("No component found for type : {}" , type);
      return false;
    }

    return itr->second(entity);
  }

  void NativeCreateComponent(Entity* entity , int32_t type) {
    OE_ASSERT(entity != nullptr , "Entity is null!");
    auto itr = create_component_funcs.find(type);
    if (itr == create_component_funcs.end()) {
      OE_WARN("No component found for type : {}" , type);
      return;
    }

    itr->second(entity);
  }

  void NativeRemoveComponent(Entity* entity , int32_t type) {
    OE_ASSERT(entity != nullptr , "Entity is null!");
    auto itr = remove_component_funcs.find(type);
    if (itr == remove_component_funcs.end()) {
      OE_WARN("No component found for type : {}" , type);
      return;
    }

    itr->second(entity);
  }

  template <typename T , typename C , T C::*member>
  using MemberAccessor = void(*)(Entity* entity , T* value);

  template <typename T>
  using Accessor = void(*)(Entity* entity , T* value);

  template <typename T>
  using CompatibleGetter = T(*)(Entity* entity);

  template <typename T>
  using CompatibleSetter = void(*)(Entity* entity , T value);

  template <typename T , typename C , T C::*member>
  static void GetMember(Entity* entity , T* value) {
    OE_ASSERT(entity != nullptr , "Entity is null!");
    *value = entity->GetComponent<C>().*member;
  }

  template <typename T , typename C , T C::*member>
  static void SetMember(Entity* entity , T* value) {
    OE_ASSERT(entity != nullptr , "Entity is null!");
    entity->GetComponent<C>().*member = *value;
  }

  template <typename T>
  void RegisterCompatibleProperty(const std::string_view klass_name , const std::string_view prop_name , ref<Assembly> assembly , 
                                  CompatibleGetter<T> getter = nullptr , CompatibleSetter<T> setter = nullptr) {
    const std::string name = fmtstr("Other.{}" , klass_name);
    Type& type = assembly->GetType(name);
    if (type.handle == -1) {
      OE_ASSERT(false , "NO C# type found for {}" , name);
    }

    if (getter != nullptr) {
      assembly->SetInternalCall(name , "Get" + std::string{ prop_name } , getter);
    }

    if (setter != nullptr) {
      assembly->SetInternalCall(name , "Set" + std::string{ prop_name } , setter);
    }
  }

  template <typename T>
  void RegisterProperty(const std::string_view klass_name , const std::string_view prop_name , ref<Assembly> assembly , 
                        Accessor<T> getter = nullptr , Accessor<T> setter = nullptr) {
    const std::string name = fmtstr("Other.{}" , klass_name);
    Type& type = assembly->GetType(name);
    if (type.handle == -1) {
      OE_ASSERT(false , "NO C# type found for {}" , name);
    }

    if (getter != nullptr) {
      assembly->SetInternalCall(name , "Get" + std::string{ prop_name } , getter);
    }

    if (setter != nullptr) {
      assembly->SetInternalCall(name , "Set" + std::string{ prop_name } , setter);
    }
  }
  
  template <typename T , typename C , T C::*member>
  void RegisterComponentMember(const std::string_view klass_name , const std::string_view member_name , ref<Assembly> assembly , 
                                MemberAccessor<T , C , member> custom_getter = nullptr , MemberAccessor<T , C , member> custom_setter = nullptr) {
    const std::string name = fmtstr("Other.{}" , klass_name);
    Type& type = assembly->GetType(name);
    if (type.handle == -1) {
      OE_ASSERT(false , "NO C# type found for {}" , name);
    }

    void* get = (void*)&GetMember<T , C , member>;
    void* set = (void*)&SetMember<T , C , member>;

    if (custom_getter != nullptr) {
      get = (void*)custom_getter;
    }

    if (custom_setter != nullptr) {
      set = (void*)custom_setter;
    }

    assembly->SetInternalCall(name , "Get" + std::string{ member_name } , get);
    assembly->SetInternalCall(name , "Set" + std::string{ member_name } , set);
  } 

} // anonymous namespace
namespace cs_script_bindings {

  void RegisterNativeComponents(ref<Assembly> assembly) {
    RegisterComponent<Relationship>(assembly);
    RegisterComponent<Transform>(assembly); 
    RegisterComponent<Mesh>(assembly);
    RegisterComponent<StaticMesh>(assembly);
    RegisterComponent<LightSource>(assembly);
    // RegisterComponent<Collider>(assembly);
    // RegisterComponent<RigidBody>(assembly);
    // RegisterComponent<Collider2D>(assembly);
    // RegisterComponent<RigidBody2D>(assembly);
    // RegisterComponent<Camera>(assembly);
    // RegisterComponent<Script>(assembly);

    RegisterInternalCallAs(assembly , "OtherObject", "NativeHasComponent" , (void*)&NativeHasComponent);
    RegisterInternalCallAs(assembly , "OtherObject", "NativeCreateComponent" , (void*)&NativeCreateComponent);
    RegisterInternalCallAs(assembly , "OtherObject", "NativeRemoveComponent" , (void*)&NativeRemoveComponent);

    RegisterFunction(
      "Scene" , "GetNativeHandle" , assembly , 
      [](uint64_t id) -> Entity* {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        if (scene == nullptr) {
          OE_ERROR("Attempting to retrieve native entity handle from invalid scene context!");
          return nullptr;
        }

        return scene->GetEntity(id);
      }
    );
    
    RegisterCompatibleProperty<dotother::NString>(
      "OtherObject" , "Name" , assembly ,
      [](Entity* entity) -> dotother::NString {
        OE_ASSERT(entity != nullptr , "Entity is null!");
        return dotother::NString::New(entity->ReadComponent<Tag>().name);
      } , 
      [](Entity* entity , dotother::NString value) {
        OE_ASSERT(entity != nullptr , "Entity is null!");
        entity->GetComponent<Tag>().name = (std::string)value;
      }
    );

    RegisterCompatibleProperty<uint64_t>(
      "Relationship" , "Parent" , assembly ,
      [](Entity* entity) -> uint64_t {
        OE_ASSERT(entity != nullptr , "Entity is null!");
        auto& relationship = entity->GetComponent<Relationship>();
        return relationship.parent.value_or(0u).Get();
      } , 
      nullptr
    );

    RegisterFunction(
      "Relationship" , "GetChildren" , assembly ,
      [](Entity* entity , uint64_t* list , uint64_t* count) {
        OE_ASSERT(entity != nullptr , "Entity is null!");
        OE_ASSERT(count != nullptr , "Count is null!");

        auto& relationship = entity->GetComponent<Relationship>();
        *count = relationship.children.size();
        
        if (list == nullptr) {
          return;
        }

        size_t idx = 0;
        for (const auto& id : relationship.children) {
          list[idx++] = id.Get();
        }
      }
    );

    RegisterComponentMember<glm::vec3 , Transform , &Transform::position>("Transform" , "Position" , assembly);
    RegisterComponentMember<glm::vec3 , Transform , &Transform::scale>("Transform" , "Scale" , assembly);
    RegisterComponentMember<glm::vec3 , Transform , &Transform::erotation>("Transform" , "Rotation" , assembly , 
      nullptr , 
      [](Entity* entity , glm::vec3* value) {
        OE_ASSERT(entity != nullptr , "Entity is null!");
        OE_ASSERT(value != nullptr , "Value is null!");
        entity->GetComponent<Transform>().erotation = *value;
        entity->GetComponent<Transform>().qrotation = glm::quat(entity->GetComponent<Transform>().erotation);
      }
    );

    RegisterFunction(
      "Transform" , "NativeRotate" , assembly ,
      [](Entity* entity , float radians , glm::vec3* axis) {
        OE_ASSERT(entity != nullptr , "Entity is null!");
        OE_ASSERT(axis != nullptr , "Axis value is null!");

        auto& transform = entity->GetComponent<Transform>();
        transform.qrotation = glm::rotate(transform.qrotation, radians, *axis);
        transform.erotation = glm::eulerAngles(transform.qrotation);
      }
    );

    RegisterProperty<glm::vec4>(
      "LightSource" , "Vector" , assembly ,
      [](Entity* entity , glm::vec4* value) {
        OE_ASSERT(entity != nullptr , "Entity is null!");
        OE_ASSERT(value != nullptr , "Value is null!");
        auto& light = entity->GetComponent<LightSource>();
        switch (light.type) {
          case LightSourceType::DIRECTION_LIGHT_SRC:
            *value = light.direction_light.direction;
          break;
          case LightSourceType::POINT_LIGHT_SRC:
            *value = light.pointlight.position;
          break;
          default:
            OE_ASSERT(false , "Unknown light source type!");
        }
      } , 
      [](Entity* entity , glm::vec4* value) {
        OE_ASSERT(entity != nullptr , "Entity is null!");
        OE_ASSERT(value != nullptr , "Value is null!");
        auto& light = entity->GetComponent<LightSource>();
        switch (light.type) {
          case LightSourceType::DIRECTION_LIGHT_SRC:
            light.direction_light.direction = *value;
          break;
          case LightSourceType::POINT_LIGHT_SRC:
            light.pointlight.position = *value;
          break;
          default:
            OE_ASSERT(false , "Unknown light source type!");
        }
      }
    );
    
    RegisterProperty<glm::vec4>(
      "LightSource" , "Color" , assembly ,
      [](Entity* entity , glm::vec4* value) {
        OE_ASSERT(entity != nullptr , "Entity is null!");
        OE_ASSERT(value != nullptr , "Value is null!");
        auto& light = entity->GetComponent<LightSource>();
        switch (light.type) {
          case LightSourceType::DIRECTION_LIGHT_SRC:
            *value = light.direction_light.color;
          break;
          case LightSourceType::POINT_LIGHT_SRC:
            *value = light.pointlight.color;
          break;
          default:
            OE_ASSERT(false , "Unknown light source type!");
        }
      } , 
      [](Entity* entity , glm::vec4* value) {
        OE_ASSERT(entity != nullptr , "Entity is null!");
        OE_ASSERT(value != nullptr , "Value is null!");
        auto& light = entity->GetComponent<LightSource>();
        switch (light.type) {
          case LightSourceType::DIRECTION_LIGHT_SRC:
            light.direction_light.color = *value;
          break;
          case LightSourceType::POINT_LIGHT_SRC:
            light.pointlight.color = *value;
          break;
          default:
            OE_ASSERT(false , "Unknown light source type!");
        }
      }
    );

    RegisterProperty<glm::vec4>(
      "LightSource" , "PointLight" , assembly ,
      [](Entity* entity , glm::vec4* value) {
        OE_ASSERT(entity != nullptr , "Entity is null!");
        OE_ASSERT(value != nullptr , "Value is null!");
        auto& light = entity->GetComponent<LightSource>();
        switch (light.type) {
          case LightSourceType::DIRECTION_LIGHT_SRC:
            *value = { 0.f , 0.f , 0.f , 0.f };
          break;
          case LightSourceType::POINT_LIGHT_SRC:
            *value = {
              light.pointlight.radius ,
              light.pointlight.constant ,
              light.pointlight.linear ,
              light.pointlight.quadratic ,
            };
          break;
          default:
            OE_ASSERT(false , "Unknown light source type!");
        }
      } , 
      [](Entity* entity , glm::vec4* value) {
        OE_ASSERT(entity != nullptr , "Entity is null!");
        OE_ASSERT(value != nullptr , "Value is null!");
        auto& light = entity->GetComponent<LightSource>();
        switch (light.type) {
          case LightSourceType::DIRECTION_LIGHT_SRC:
          break;
          case LightSourceType::POINT_LIGHT_SRC:
            light.pointlight.radius = value->x;
            light.pointlight.constant = value->y;
            light.pointlight.linear = value->z;
            light.pointlight.quadratic = value->w;
          break;
          default:
            OE_ASSERT(false , "Unknown light source type!");
        }
      }
    );
  }

} // namespace cs_script_bindings
} // namespace other
