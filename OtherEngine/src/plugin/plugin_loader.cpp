/**
 * \file plugin/plugin_loader.cpp
 */
#include "plugin/plugin_loader.hpp"

#include "core/platform.hpp"

#ifdef OE_WINDOWS
  #include "windows/windows_plugin_loader.hpp"
#endif

namespace other {

  Scope<PluginLoader> GetPluginLoader(const std::string& path) {
#ifdef OE_WINDOWS
    return NewScope<WindowsPluginLoader>(path);
#else
    return nullptr;
#endif
  }

} // namespace other
