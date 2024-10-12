/**
 * \file scripting/cs/cs_component_bindings.hpp
 **/
#ifndef OTHER_ENGINE_CS_COMPONENT_BINDINGS_HPP
#define OTHER_ENGINE_CS_COMPONENT_BINDINGS_HPP

#include <hosting/assembly.hpp>

using dotother::ref;
using dotother::Assembly;

namespace other {
namespace cs_script_bindings {

  void RegisterNativeComponents(ref<Assembly> assembly);

} // namespace cs_script_bindings 
} // namespace other

#endif //!OTHER_ENGINE_CS_COMPONENT_BINDINGS_HPP
