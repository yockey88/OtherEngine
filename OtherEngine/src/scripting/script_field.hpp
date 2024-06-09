/**
 * \file scripting/script_field.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_FIELD_HPP
#define OTHER_ENGINE_SCRIPT_FIELD_HPP

#include "core/uuid.hpp"
#include "core/value.hpp"

namespace other {

  struct ScriptField {
    UUID id;
    std::string name;
    Value value;
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_FIELD_HPP
