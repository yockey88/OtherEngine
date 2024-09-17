/**
 * \file scripting/cs/cs_script_bindings.cpp
 **/
#include "scripting/cs/cs_script_bindings.hpp"

#include "core/type_data.hpp"

#include "input/mouse.hpp"
#include "input/keyboard.hpp"

#include "ecs/components/tag.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/rigid_body_2d.hpp"

#include "scripting/cs/native_functions/cs_native_helpers.hpp"
#include "scripting/cs/native_functions/cs_native_logging.hpp"
#include "scripting/cs/native_functions/cs_native_input.hpp"
#include "scripting/cs/native_functions/cs_native_rendering.hpp"
#include "scripting/cs/native_functions/cs_native_entity.hpp"
#include "scripting/cs/native_functions/cs_native_core_components.hpp"
#include "scripting/cs/native_functions/cs_native_physics_components.hpp"
#include "scripting/cs/native_functions/cs_native_model_factory.hpp"
#include "scripting/cs/native_functions/cs_native_helpers.hpp"
#include <mono/metadata/loader.h>

namespace other {

  static FunctionMaps* function_maps = nullptr;

  template <ComponentType C>
  std::map<MonoType* , std::function<bool(Entity* , C*)>> FunctionMaps::component_getters{};

  template <ComponentType C>
  std::map<MonoType* , std::function<void(Entity* , C*)>> FunctionMaps::component_setters{};

  template <ComponentType C>
  static void RegisterComponentType(MonoImage* asm_image) {
    std::string_view name = TypeData<C , true>().Name();

    std::string full_name = "Other." + std::string(name);

    MonoType* mtype = mono_reflection_type_from_name(full_name.data() , asm_image);
    if (mtype == nullptr) {
      OE_ERROR("Failed to bind type {} to C# assembly" , full_name);
      return;
    }

    function_maps->component_builders[mtype] = [](Entity* ent) { ent->AddComponent<C>(); };
    function_maps->component_checkers[mtype] = [](Entity* ent) -> bool { return ent->HasComponent<C>(); };
    function_maps->component_destroyers[mtype] = [](Entity* ent) { 
      if constexpr (std::is_same_v<C , Tag>) {
        OE_WARN("Attempting to remove tag component from {}" , ent->Name());
        return;
      } else if constexpr (std::is_same_v<C , Transform>) {
        OE_WARN("Attempting to remove transform component from {}" , ent->Name());
        return;
      } else if constexpr (std::is_same_v<C , Relationship>) {
        OE_WARN("Attempting to remove relationship component from {}" , ent->Name());
        return;
      }
      ent->RemoveComponent<C>(); 
    };

    function_maps->component_getters<C>[mtype] = [&name](Entity* ent , C* component) -> bool {
      if (!ent->HasComponent<C>()) {
        OE_ERROR("Entity {} does not have component {}" , ent->Name() , std::string{ name });

        *component = C();
        return false;
      }

      const auto& ent_comp = ent->ReadComponent<C>();
      *component = ent_comp;
      return true;
    };

    function_maps->component_setters<C>[mtype] = [&name](Entity* ent , C* component) {
      if (!ent->HasComponent<C>()) {
        /// should we add the component?
        OE_WARN("Setting component {} on entity without it {}" , std::string{ name  } , ent->Name());
        return;
      }

      if (component == nullptr) {
        OE_ERROR("Attempted to set entity {} component {} to null" , ent->Name() , std::string{ name });
      }

      auto& ent_component = ent->GetComponent<C>();
      ent_component = *component;
    };
  }

  template <ComponentType C>
  static void UnloadComponentFunctions() {
    function_maps->component_getters<C>.clear();
    function_maps->component_setters<C>.clear();
  }

  void CsScriptBindings::InitializeBindings(MonoImage* asm_image) {
    if (function_maps != nullptr) {
      delete function_maps;
    }

    function_maps = new FunctionMaps;

    RegisterComponentType<Tag>(asm_image);
    RegisterComponentType<Transform>(asm_image);
    RegisterComponentType<Relationship>(asm_image);
    RegisterComponentType<Script>(asm_image);
    RegisterComponentType<Mesh>(asm_image);
    RegisterComponentType<Camera>(asm_image);
    RegisterComponentType<RigidBody2D>(asm_image);
    RegisterComponentType<StaticMesh>(asm_image);

    RegisterNativeFunctions(asm_image);
  }
      
  void CsScriptBindings::ShutdownBindings() {
    function_maps->component_destroyers.clear();
    function_maps->component_checkers.clear();
    function_maps->component_builders.clear();

    UnloadComponentFunctions<Tag>();
    UnloadComponentFunctions<Transform>();
    UnloadComponentFunctions<Relationship>();
    UnloadComponentFunctions<Script>();
    UnloadComponentFunctions<Mesh>();
    UnloadComponentFunctions<Camera>();
    UnloadComponentFunctions<RigidBody2D>();
    UnloadComponentFunctions<StaticMesh>();

    delete function_maps;
    function_maps = nullptr;
  }

  std::function<void(Entity*)> CsScriptBindings::GetBuilder(MonoType* type) {
    OE_ASSERT(function_maps != nullptr , "Attempting to access uninitialized C# bindings");
    auto itr = function_maps->component_builders.find(type);
    if (itr == function_maps->component_builders.end()) {
      return nullptr;
    }

    return itr->second;
  }

  std::function<bool(Entity*)> CsScriptBindings::GetChecker(MonoType* type) {
    OE_ASSERT(function_maps != nullptr , "Attempting to access uninitialized C# bindings");
    auto itr = function_maps->component_checkers.find(type);
    if (itr == function_maps->component_checkers.end()) {
      return nullptr;
    }

    return itr->second;
  }

  std::function<void(Entity*)> CsScriptBindings::GetDestroyer(MonoType* type) {
    OE_ASSERT(function_maps != nullptr , "Attempting to access uninitialized C# bindings");
    auto itr = function_maps->component_destroyers.find(type);
    if (itr == function_maps->component_destroyers.end()) {
      return nullptr;
    }

    return itr->second;
  }

  void CsScriptBindings::RegisterNativeFunctions(MonoImage* asm_image) {
#define BIND_CS_READWRITE_PROPERTY(name, vt , c , m)                                                                                      \
  cs_script_bindings::NativeRetrieveThing<vt, c, &c::m>::type NativeGet##name = &cs_script_bindings::NativeRetrieveThingFn<vt, c, &c::m>; \
  cs_script_bindings::NativeSetThing<vt, c, &c::m>::type NativeSet##name = &cs_script_bindings::NativeSetThingFn<vt, c, &c::m>;

#define _REGISTER_NATIVE_FUNCTION(class_name , comp_name , vt , m)                                         \
    {                                                                                                      \
      BIND_CS_READWRITE_PROPERTY(comp_name, vt , comp_name , m);                                           \
      mono_add_internal_call("Other." #class_name "::NativeGet" #comp_name , (void*)NativeGet##comp_name); \
      mono_add_internal_call("Other." #class_name "::NativeSet" #comp_name , (void*)NativeSet##comp_name); \
    }

#define REGISTER_NATIVE_FUNCTION(class_name , name) \
    mono_add_internal_call("Other." #class_name "::" #name , (void*)cs_script_bindings::name);

  /// These functions have to be routed a different path because of some sort of data transformation that needs to occur
  /// 
  /// For example the logging functions get passed MonoStrings from the .NET runtime which need to be translated to c-strings
  ///
  /// Other functions might need to access some non-static state such as the scene/app context which they will do through the 
  ///   ScriptEngine class
  ///
  /// Still other functions might require the transfer of non-primitive types that are given to the .NET runtime as out parameters
  ///   like the MouseXXXPos calls, which have glm::vec2* parameters natively, but are passed Other.Vec2 out parameters from the .NET runtime

  REGISTER_NATIVE_FUNCTION(Transform , NativeGetScale);
  REGISTER_NATIVE_FUNCTION(Transform , NativeSetScale);

  REGISTER_NATIVE_FUNCTION(Transform , NativeGetPosition);
  REGISTER_NATIVE_FUNCTION(Transform , NativeSetPosition);

  REGISTER_NATIVE_FUNCTION(Transform , NativeGetRotation);
  REGISTER_NATIVE_FUNCTION(Transform , NativeSetRotation);
  REGISTER_NATIVE_FUNCTION(Transform , NativeRotateObject);

  REGISTER_NATIVE_FUNCTION(Transform , NativeGetQuaternion);
  REGISTER_NATIVE_FUNCTION(Transform , NativeSetQuaternion);
  // _REGISTER_NATIVE_FUNCTION(Transform , Quaternion, glm::quat, qrotation);

  REGISTER_NATIVE_FUNCTION(Logger , WriteLine); 
  REGISTER_NATIVE_FUNCTION(Logger , WriteTrace); 
  REGISTER_NATIVE_FUNCTION(Logger , WriteDebug); 
  REGISTER_NATIVE_FUNCTION(Logger , WriteInfo); 
  REGISTER_NATIVE_FUNCTION(Logger , WriteWarning); 
  REGISTER_NATIVE_FUNCTION(Logger , WriteError); 
  REGISTER_NATIVE_FUNCTION(Logger , WriteCritical); 

  REGISTER_NATIVE_FUNCTION(Mouse , MousePos);
  REGISTER_NATIVE_FUNCTION(Mouse , MousePreviousPos);
  REGISTER_NATIVE_FUNCTION(Mouse , MouseDeltaPos);

  REGISTER_NATIVE_FUNCTION(Scene , NativeGetName);
  REGISTER_NATIVE_FUNCTION(Scene , NativeGetObjectByName);
  REGISTER_NATIVE_FUNCTION(Scene , NativeHasObjectById);
  REGISTER_NATIVE_FUNCTION(Scene , NativeHasObjectByName);


  REGISTER_NATIVE_FUNCTION(Scene , NativeGetParent);
  REGISTER_NATIVE_FUNCTION(Scene , NativeGetNumChildren);
  REGISTER_NATIVE_FUNCTION(Scene , NativeGetChildren);

  REGISTER_NATIVE_FUNCTION(Scene , NativeGetPhysicsBodyType);
  REGISTER_NATIVE_FUNCTION(Scene , NativeGetMass);
  REGISTER_NATIVE_FUNCTION(Scene , NativeGetLinearDrag);
  REGISTER_NATIVE_FUNCTION(Scene , NativeGetAngularDrag);
  REGISTER_NATIVE_FUNCTION(Scene , NativeGetGravityScale);
  REGISTER_NATIVE_FUNCTION(Scene , NativeGetFixedRotation);
  REGISTER_NATIVE_FUNCTION(Scene , NativeGetBullet);
  
  REGISTER_NATIVE_FUNCTION(Scene , NativeGet2DColliderOffset);
  REGISTER_NATIVE_FUNCTION(Scene , NativeGet2DColliderSize);
  REGISTER_NATIVE_FUNCTION(Scene , NativeGet2DColliderDensity);
  REGISTER_NATIVE_FUNCTION(Scene , NativeGet2DColliderFriction);

  REGISTER_NATIVE_FUNCTION(Scene , NativeGetStaticMeshHandle);
  REGISTER_NATIVE_FUNCTION(Scene , NativeGetStaticMeshMaterial);

  REGISTER_NATIVE_FUNCTION(Scene , NativeAddComponent);
  REGISTER_NATIVE_FUNCTION(Scene , NativeHasComponent);
  REGISTER_NATIVE_FUNCTION(Scene , NativeRemoveComponent);
  
  REGISTER_NATIVE_FUNCTION(Scene , NativeSetPhysicsBodyType);
  REGISTER_NATIVE_FUNCTION(Scene , NativeSetMass);
  REGISTER_NATIVE_FUNCTION(Scene , NativeSetLinearDrag);
  REGISTER_NATIVE_FUNCTION(Scene , NativeSetAngularDrag);
  REGISTER_NATIVE_FUNCTION(Scene , NativeSetGravityScale);
  REGISTER_NATIVE_FUNCTION(Scene , NativeSetFixedRotation);
  REGISTER_NATIVE_FUNCTION(Scene , NativeSetBullet);
  
  REGISTER_NATIVE_FUNCTION(Scene , NativeSet2DColliderOffset);
  REGISTER_NATIVE_FUNCTION(Scene , NativeSet2DColliderSize);
  REGISTER_NATIVE_FUNCTION(Scene , NativeSet2DColliderDensity);
  REGISTER_NATIVE_FUNCTION(Scene , NativeSet2DColliderFriction);

  REGISTER_NATIVE_FUNCTION(Scene , NativeSetStaticMeshHandle);
  REGISTER_NATIVE_FUNCTION(Scene , NativeSetStaticMeshMaterial);

  REGISTER_NATIVE_FUNCTION(ModelFactory , NativeCreateBox);

#undef REGISTER_NATIVE_FUNCTION

#define REGISTER_NATIVE_FUNCTION(class_name , name) \
   mono_add_internal_call("Other." #class_name "::" #name , (void*)class_name::name);

   /// These functions can be bound directly to static class functions 

   REGISTER_NATIVE_FUNCTION(Keyboard , FramesHeld);
   REGISTER_NATIVE_FUNCTION(Keyboard , Pressed); 
   REGISTER_NATIVE_FUNCTION(Keyboard , Blocked); 
   REGISTER_NATIVE_FUNCTION(Keyboard , Held); 
   REGISTER_NATIVE_FUNCTION(Keyboard , Down); 
   REGISTER_NATIVE_FUNCTION(Keyboard , Released); 

   REGISTER_NATIVE_FUNCTION(Mouse , SnapToCenter);
   REGISTER_NATIVE_FUNCTION(Mouse , FreeCursor);
   REGISTER_NATIVE_FUNCTION(Mouse , LockCursor);
   REGISTER_NATIVE_FUNCTION(Mouse , GetX);
   REGISTER_NATIVE_FUNCTION(Mouse , GetY);
   REGISTER_NATIVE_FUNCTION(Mouse , PreviousX);
   REGISTER_NATIVE_FUNCTION(Mouse , PreviousY);
   REGISTER_NATIVE_FUNCTION(Mouse , GetDX);
   REGISTER_NATIVE_FUNCTION(Mouse , GetDY);
   REGISTER_NATIVE_FUNCTION(Mouse , InWindow);
   REGISTER_NATIVE_FUNCTION(Mouse , FramesHeld);
   REGISTER_NATIVE_FUNCTION(Mouse , Pressed);
   REGISTER_NATIVE_FUNCTION(Mouse , Blocked);
   REGISTER_NATIVE_FUNCTION(Mouse , Held);
   REGISTER_NATIVE_FUNCTION(Mouse , Released);

#undef REGISTER_NATIVE_FUNCTION
  }

} // namespace other
