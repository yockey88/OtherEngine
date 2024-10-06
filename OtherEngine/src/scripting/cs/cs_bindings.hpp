/**
 * \file scripting/cs/cs_bindings.hpp
 **/
#ifndef OTHER_ENGINE_CS_BINDINGS_HPP
#define OTHER_ENGINE_CS_BINDINGS_HPP

#include <hosting/assembly.hpp>

using dotother::ref;
using dotother::Assembly;

namespace other {
namespace cs_script_bindings {

  void RegisterInternalCalls(ref<Assembly> assembly);

} // namespace cs_script_bindings
} // namespace other

#endif // !OTHER_ENGINE_CS_BINDINGS_HPP