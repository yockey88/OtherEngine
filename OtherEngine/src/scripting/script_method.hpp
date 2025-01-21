/**
 * \file scripting/script_method.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_METHOD_HPP
#define OTHER_ENGINE_SCRIPT_METHOD_HPP

#include <string>

#include "core/uuid.hpp"

namespace other {

  struct ScriptMethod {
    UUID id;
    std::string name;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_SCRIPT_METHOD_HPP