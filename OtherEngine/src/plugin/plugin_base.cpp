/**
 * \file plugin/plugin.cpp
 */
#include "plugin/plugin.hpp"

#include "core/engine.hpp"

namespace other {

  Engine* Plugin::GetEngine() {
    return engine;
  }

} // namespace other
