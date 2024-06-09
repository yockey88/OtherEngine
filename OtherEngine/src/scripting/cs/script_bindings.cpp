/**
 * \file scripting/cs/script_bindings.cpp
 **/
#include "scripting/cs/script_bindings.hpp"

#include "core/type_data.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/mesh.hpp"
#include "scripting/cs/native_functions/native_logging.hpp"

namespace other {

  static FunctionMaps* function_maps = nullptr;

  template <component_type C>
  std::map<MonoType* , std::function<bool(Entity* , C*)>> FunctionMaps::component_getters{};

  template <component_type C>
  std::map<MonoType* , std::function<void(Entity* , C*)>> FunctionMaps::component_setters{};

  template <component_type C>
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

  void CsScriptBindings::InitializeBindings(MonoImage* asm_image) {
    if (function_maps == nullptr) {
      function_maps = new FunctionMaps;
    }

    RegisterComponentType<Tag>(asm_image);
    RegisterComponentType<Transform>(asm_image);
    RegisterComponentType<Relationship>(asm_image);
    RegisterComponentType<Script>(asm_image);
    RegisterComponentType<Mesh>(asm_image);

    RegisterNativeFunctions(asm_image);
  }
      
  void CsScriptBindings::ShutdownBindings() {
    function_maps->component_destroyers.clear();
    function_maps->component_checkers.clear();
    function_maps->component_builders.clear();
    delete function_maps;
  }

  void CsScriptBindings::RegisterNativeFunctions(MonoImage* asm_image) {
#define REGISTER_NATIVE_FUNCTION(class_name , name) \
    mono_add_internal_call("Other." #class_name "::" #name , (void*)script_bindings::name);

   REGISTER_NATIVE_FUNCTION(Logger , WriteLine); 
   REGISTER_NATIVE_FUNCTION(Logger , WriteTrace); 
   REGISTER_NATIVE_FUNCTION(Logger , WriteDebug); 
   REGISTER_NATIVE_FUNCTION(Logger , WriteWarning); 
   REGISTER_NATIVE_FUNCTION(Logger , WriteError); 
   REGISTER_NATIVE_FUNCTION(Logger , WriteCritical); 

#undef REGISTER_NATIVE_FUNCTION
  }

} // namespace other
