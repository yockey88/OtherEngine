/**
 * \file platform/windows/windows_plugin_loader.hpp
 */
#ifndef OTHER_ENGINE_WINDOWS_PLUGIN_LOADER_HPP
#define OTHER_ENGINE_WINDOWS_PLUGIN_LOADER_HPP

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "plugin/plugin_loader.hpp"

namespace other {

  class WindowsPluginLoader : public PluginLoader {
    public:
      WindowsPluginLoader(const std::string& path) 
        : PluginLoader(path) {}
      virtual ~WindowsPluginLoader() = default;

      virtual Plugin* Load(Engine* engine) override;
      virtual void Unload() override;

    private:
      HMODULE module = nullptr;
  };

} // namespace other

#endif // !OTHER_ENGINE_WINDOWS_PLUGIN_LOADER_HPP
