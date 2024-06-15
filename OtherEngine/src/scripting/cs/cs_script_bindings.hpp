/**
 * \file scripting/cs/cs_script_bindings.hpp
 **/
#ifndef OTHER_ENGINE_CS_SCRIPT_BINDINGS_HPP
#define OTHER_ENGINE_CS_SCRIPT_BINDINGS_HPP

#include <map>
#include <functional>

#include <glm/glm.hpp>
#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>

#include "ecs/entity.hpp"
#include "ecs/component.hpp"

namespace other {

  struct FunctionMaps {
    std::map<MonoType* , std::function<void(Entity*)>> component_builders{};
    std::map<MonoType* , std::function<bool(Entity*)>> component_checkers{};
    std::map<MonoType* , std::function<void(Entity*)>> component_destroyers{};

    template <component_type C>
    static std::map<MonoType* , std::function<bool(Entity* , C*)>> component_getters;

    template <component_type C>
    static std::map<MonoType* , std::function<void(Entity* , C*)>> component_setters;
  };

  class CsScriptBindings {
    public:
      static void InitializeBindings(MonoImage* asm_image);
      static void ShutdownBindings();

    private:
      static void RegisterNativeFunctions(MonoImage* asm_image);
  };

} // namespace other

#endif // !OTHER_ENGINE_CS_SCRIPT_BINDINGS_HPP
