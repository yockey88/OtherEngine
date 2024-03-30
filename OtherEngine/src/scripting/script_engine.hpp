/**
 * \file scripting\script_engine.hpp
 */
#ifndef OTHER_ENGINE_SCRIPT_ENGINE_HPP
#define OTHER_ENGINE_SCRIPT_ENGINE_HPP

#include <string>
#include <map>

#include "core/defines.hpp"
#include "core/uuid.hpp"
#include "core/config.hpp"
#include "plugin/plugin_loader.hpp"
#include "scripting/language_module.hpp"

namespace other {

  class Engine;
  class App;
  class LanguageModule;

  struct LanguageModuleMetadata {
    UUID id;
    std::string path;
    std::string name;
    LanguageModule* module = nullptr;
    Scope<PluginLoader> loader = nullptr;
  };

  class ScriptEngine {
    public:
      static void Initialize(Engine* engine , const ConfigTable& config);
      static void Shutdown();
      
      static void LoadModule(const std::string& name , const std::string& path);
      static void UnloadModule(const std::string& name);

      static LanguageModule* GetModule(const std::string& name);
      static LanguageModule* GetModule(const UUID& id);

      static void SetAppContext(App* app);

      static const std::map<UUID , LanguageModuleMetadata>& GetModules() { return language_modules; }

    private:
      static Engine* engine_handle;
      static App* app_context;
      static ConfigTable config;

      static std::vector<std::string> module_paths;
      static std::map<UUID , LanguageModuleMetadata> language_modules;

      static void LoadDefaultModules();

      static void ReinitializeModule(const UUID& id);
      static void ReinitializeAllModules();
      
      static void UnloadModule(const UUID& id);
      static void UnloadAllModules();
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_ENGINE_HPP
