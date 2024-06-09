/**
 * \file scripting/script_module.cpp
 **/
#include "scripting/script_module.hpp"

namespace other {

  bool ScriptModule::HasChanged() const {
    return changed_on_disk;
  }

} // namespace other
