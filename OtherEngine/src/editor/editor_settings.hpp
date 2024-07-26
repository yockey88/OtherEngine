/**
 * \file editor/editor_settings.hpp
 **/
#ifndef OTHER_ENGINE_EDITOR_SETTINGS_HPP
#define OTHER_ENGINE_EDITOR_SETTINGS_HPP

#include <cstdint>

#include <imgui/imgui.h>

#include "rendering/ui/ui_window.hpp"

namespace other {

  struct EditorSettings {
    uint16_t thumbnail_size = 100;

    static EditorSettings& Get();
  };

  class SettingsWindow : public UIWindow {
    public:
      SettingsWindow();
      virtual ~SettingsWindow() override {}

    private:
      void EditorSettings();
  };

} // namespace other

#endif // !OTHER_ENGINE_EDITOR_SETTINGS_HPP
