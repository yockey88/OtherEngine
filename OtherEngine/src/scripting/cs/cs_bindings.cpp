/**
 * \file scripting/cs/cs_bindings.cpp
 **/
#include "scripting/cs/cs_bindings.hpp"

#include <hosting/type.hpp>

#include "input/keyboard.hpp"
#include "input/mouse.hpp"

#include "scripting/cs/cs_component_bindings.hpp"
#include "scripting/cs/cs_entity_bindings.hpp"
#include "scripting/cs/cs_logging_bindings.hpp"
#include "scripting/cs/cs_physics_bindings.hpp"
#include "scripting/cs/cs_register_internal_call.hpp"
#include "scripting/cs/cs_scene_bindings.hpp"

namespace other {
  namespace cs_script_bindings {

    void RegisterEngineBindings(ref<Assembly>);

    void RegisterInternalCalls(ref<Assembly> assembly) {
      RegisterEngineBindings(assembly);
      RegisterNativeComponents(assembly);
      RegisterEntityBindings(assembly);
      RegisterSceneFunctions(assembly);

      RegisterInternalCallAs(assembly, "Logger", "Write", (void*)&cs_script_bindings::Write);
    }

    void RegisterEngineBindings(ref<Assembly> assembly) {
      RegisterFunction("Keyboard", "IsKeyPressed", assembly, (void*)&Keyboard::Pressed);
      RegisterFunction("Keyboard", "IsKeyBlocked", assembly, (void*)&Keyboard::Blocked);
      RegisterFunction("Keyboard", "IsKeyHeld", assembly, (void*)&Keyboard::Held);
      RegisterFunction("Keyboard", "IsKeyDown", assembly, (void*)&Keyboard::Down);
      RegisterFunction("Keyboard", "IsKeyReleased", assembly, (void*)&Keyboard::Released);

      RegisterFunction("Keyboard", "IsLCtrlLayer", assembly, (void*)&Keyboard::LCtrlLayer);
      RegisterFunction("Keyboard", "IsRCtrlLayer", assembly, (void*)&Keyboard::RCtrlLayer);
      RegisterFunction("Keyboard", "IsLShiftLayer", assembly, (void*)&Keyboard::LShiftLayer);
      RegisterFunction("Keyboard", "IsRShiftLayer", assembly, (void*)&Keyboard::RShiftLayer);
      RegisterFunction("Keyboard", "IsLAltLayer", assembly, (void*)&Keyboard::LAltLayer);
      RegisterFunction("Keyboard", "IsRAltLayer", assembly, (void*)&Keyboard::RAltLayer);
      RegisterFunction("Keyboard", "IsLCtrlShiftLayer", assembly, (void*)&Keyboard::LCtrlShiftLayer);
      RegisterFunction("Keyboard", "IsRCtrlShiftLayer", assembly, (void*)&Keyboard::RCtrlShiftLayer);
      RegisterFunction("Keyboard", "IsLCtrlAltLayer", assembly, (void*)&Keyboard::LCtrlAltLayer);
      RegisterFunction("Keyboard", "IsRCtrlAltLayer", assembly, (void*)&Keyboard::RCtrlAltLayer);
      RegisterFunction("Keyboard", "IsLCtrlAltShiftLayer", assembly, (void*)&Keyboard::LCtrlAltShiftLayer);
      RegisterFunction("Keyboard", "IsRCtrlAltShiftLayer", assembly, (void*)&Keyboard::RCtrlAltShiftLayer);

      RegisterFunction("Keyboard", "IsLCtrlLayerKey", assembly, (void*)&Keyboard::LCtrlLayerKey);
      RegisterFunction("Keyboard", "IsRCtrlLayerKey", assembly, (void*)&Keyboard::RCtrlLayerKey);

      RegisterFunction("Keyboard", "IsLAltLayerKey", assembly, (void*)&Keyboard::LAltLayerKey);
      RegisterFunction("Keyboard", "IsRAltLayerKey", assembly, (void*)&Keyboard::RAltLayerKey);

      /// keyboard/mouse bindings

      /// physics bindings
      RegisterFunction("Physics", "NativeRaycast", assembly, (void*)&cs_script_bindings::NativeRaycast);
    }

  }  // namespace cs_script_bindings
}  // namespace other