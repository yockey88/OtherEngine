/**
 * \file plugin/plugin_base.hpp
*/
#ifndef OTHER_ENGINE_PLUGIN_BASE_HPP
#define OTHER_ENGINE_PLUGIN_BASE_HPP

#include <string>

#include "core/engine.hpp"

namespace other {

  class Plugin {
    public:
      Plugin(Engine* engine) 
        : engine(engine) {}
      virtual ~Plugin() {}

    protected:
      Engine* GetEngine();
    
    private:
      Engine* engine = nullptr;
  };

} // namespace other

#endif // !OTHER_ENGINE_PLUGIN_BASE_HPP
