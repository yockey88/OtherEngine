/**
 * \file plugin/plugin_base.hpp
*/
#ifndef OTHER_ENGINE_PLUGIN_BASE_HPP
#define OTHER_ENGINE_PLUGIN_BASE_HPP

#include <string>

#include "core/logger.hpp"
#include "core/engine.hpp"

namespace other {

  class Plugin {
    public:
      Plugin(Engine* engine) 
        : engine(engine) {}
      virtual ~Plugin() {}

    protected:
      Engine* GetEngine();

      template<typename... Args>
      void LogTrace(const std::string& fmt , Args&&... args) {
        engine->GetLog()->GetLogger()->trace(fmt::runtime(fmt) , std::forward<Args>(args)...);
      }

      template<typename... Args>
      void LogDebug(const std::string& fmt , Args&&... args) {
        engine->GetLog()->GetLogger()->debug(fmt::runtime(fmt) , std::forward<Args>(args)...);
      }

      template<typename... Args>
      void LogInfo(const std::string& fmt , Args&&... args) {
        engine->GetLog()->GetLogger()->info(fmt::runtime(fmt) , std::forward<Args>(args)...);
      }

      template<typename... Args>
      void LogWarn(const std::string& fmt , Args&&... args) {
        engine->GetLog()->GetLogger()->warn(fmt::runtime(fmt) , std::forward<Args>(args)...);
      }

      template<typename... Args>
      void LogError(const std::string& fmt , Args&&... args) {
        engine->GetLog()->GetLogger()->error(fmt::runtime(fmt) , std::forward<Args>(args)...);
      }

      template<typename... Args>
      void LogCritical(const std::string& fmt , Args&&... args) {
        engine->GetLog()->GetLogger()->critical(fmt::runtime(fmt) , std::forward<Args>(args)...);
      }
    
    private:
      Engine* engine = nullptr;
  };

} // namespace other

#endif // !OTHER_ENGINE_PLUGIN_BASE_HPP
