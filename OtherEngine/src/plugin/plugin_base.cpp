/**
 * \file plugin/plugin.cpp
 */
#include "plugin/plugin.hpp"

#include "engine/engine.hpp"

namespace other {

  Engine* Plugin::GetEngine() {
    return engine;
  }

}  // namespace other
