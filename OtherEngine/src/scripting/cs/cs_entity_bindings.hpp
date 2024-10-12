/**
 * \file scripting/cs/cs_entity_bindings.hpp
 **/
#ifndef CS_ENTITY_BINDINGS_HPP
#define CS_ENTITY_BINDINGS_HPP

#include <hosting/assembly.hpp>

using dotother::ref;
using dotother::Assembly;

namespace other {
namespace cs_script_bindings {

  void RegisterEntityBindings(ref<Assembly> assembly);

} // namespace cs_script_bindings
} // namespace other

#endif // !CS_ENTITY_BINDINGS_HPP
