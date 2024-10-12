/**
 * \file scripting/cs/cs_bindings.cpp
 **/
#include "scripting/cs/cs_bindings.hpp"

#include <hosting/type.hpp>

#include "scripting/cs/cs_register_internal_call.hpp"
#include "scripting/cs/cs_entity_bindings.hpp"
#include "scripting/cs/cs_component_bindings.hpp"
#include "scripting/cs/cs_logging_bindings.hpp"
#include "scripting/cs/cs_scene_bindings.hpp"

namespace other {
namespace cs_script_bindings {

  void RegisterInternalCalls(ref<Assembly> assembly) {
    RegisterNativeComponents(assembly);
    RegisterEntityBindings(assembly);
    RegisterSceneFunctions(assembly);
  
    RegisterInternalCallAs(assembly , "Logger" , "Write" , (void*)&cs_script_bindings::Write);
  }

} // namespace cs_script_bindings
} // namespace other