/**
 * \file scripting\script_engine.hpp
 */
#ifndef OTHER_ENGINE_SCRIPT_ENGINE_HPP
#define OTHER_ENGINE_SCRIPT_ENGINE_HPP

namespace other {

  class ScriptEngine {
    public:
      static void Init();
      static void Shutdown();
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_ENGINE_HPP
