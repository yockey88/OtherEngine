/**
 * \file scripting/cs/cs_logging_bindings.hpp
 **/
#ifndef OTHER_ENGINE_CS_LOGGING_BINDINGS_HPP
#define OTHER_ENGINE_CS_LOGGING_BINDINGS_HPP

#include <hosting/native_string.hpp>

#include "core/logger.hpp"

namespace other {
namespace cs_script_bindings {

  void Write(const dotother::NString string , Logger::Level level);

} // namespace cs_script_bindings
} // namespace other

#endif // !OTHER_ENGINE_CS_LOGGING_BINDINGS_HPP
