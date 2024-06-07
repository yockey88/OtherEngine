/**
 * \file plugin/plugin_loader.hpp
 */
#ifndef OTHER_ENGINE_PLUGIN_LOADER_HPP
#define OTHER_ENGINE_PLUGIN_LOADER_HPP

#include "plugin/plugin.hpp"

namespace other {

  class PluginLoader {
    public:
      PluginLoader(const std::string& path) 
        : path(path) {}
      virtual ~PluginLoader() = default;

      virtual Plugin* Load(Engine* engine) = 0;
      virtual void Unload() = 0;

    protected:
      std::string path;
      Plugin* plugin = nullptr;
  };

  Scope<PluginLoader> GetPluginLoader(const std::string_view path);

} // namespace other

#endif // !OTHER_ENGINE_PLUGIN_LOADER_HPP
