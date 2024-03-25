/**
 * \file platform/windows/windows_plugin_loader.cpp
 */
#include "windows/windows_plugin_loader.hpp"

#include "plugin/plugin.hpp"
#include <filesystem>

namespace other {

  typedef Plugin*(*PluginCreateFunc)(Engine* engine);
  typedef void(*PluginDestroyFunc)(Plugin* plugin);

  Plugin* WindowsPluginLoader::Load(Engine* engine) {
    auto full_path = std::filesystem::absolute(path);
    auto rpath = full_path.string();

    std::replace(rpath.begin() , rpath.end() , '/' , '\\');
    std::wstring wpath(rpath.begin() , rpath.end());

    OE_DEBUG("Loading plugin: {0}" , rpath);

    if (!std::filesystem::exists(wpath)) {
      OE_ERROR("Failed to load plugin, rpath does not exist: {0}" , rpath);
      return nullptr;
    } 

    OE_DEBUG("Plugin path exists: {0}" , rpath);

    module = LoadLibraryA(rpath.c_str());
    if (module == nullptr) {
      OE_ERROR("Failed to load plugin, path does not exist: {} | {}" , path , GetLastError());
      return nullptr;
    }

    PluginCreateFunc create_func = (PluginCreateFunc)GetProcAddress(module , "CreatePlugin");
    if (create_func == nullptr) {
      OE_ERROR("Failed to load plugin {} does not have load symbol: CreatePlugin" , path);
      FreeLibrary(module);
      module = nullptr;
      return nullptr;
    }

    Plugin* plugin = create_func(engine);
    if (plugin == nullptr) {
      OE_ERROR("Failed to load plugin {} create function returned nullptr" , path);
      FreeLibrary(module);
      module = nullptr;
      return nullptr;
    }

    return plugin;
  }

  void WindowsPluginLoader::Unload() {
    if (module == nullptr) {
      return;
    }

    PluginDestroyFunc destroy_func = (PluginDestroyFunc)GetProcAddress(module , "DestroyPlugin");
    if (destroy_func == nullptr) {
      OE_ERROR("Failed to unload plugin, does not have unload symbol: {0}" , path);
      return;
    }

    destroy_func(plugin);
    FreeLibrary(module);
  }

} // namespace other
