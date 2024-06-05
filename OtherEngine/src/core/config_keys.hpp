/**
 * \file core/config_keys.hpp
 **/
#ifndef OTHER_ENGINE_CONFIG_KEYS_HPP
#define OTHER_ENGINE_CONFIG_KEYS_HPP

#include <string_view>

#include "core/defines.hpp"

namespace other {

  /// section keys
  constexpr static std::string_view kProjectSection = "PROJECT";
  constexpr static uint64_t kProjectSectionHash = FNV(kProjectSection);

  constexpr static std::string_view kWindowSection = "WINDOW";
  constexpr static uint64_t kWindowSectionHash = FNV(kWindowSection);
  
  constexpr static std::string_view kWindowFlagsSection = "WINDOW.FLAGS";
  constexpr static uint64_t kWindowFlagsSectionHash = FNV(kWindowSection);

  constexpr static std::string_view kRendererSection = "RENDERER";
  constexpr static uint64_t kRendererSectionHash = FNV(kRendererSection);

  constexpr static std::string_view kUiSection = "UI";
  constexpr static uint64_t kUiSectionHash = FNV(kUiSection);

  constexpr static std::string_view kLogSection = "LOG";
  constexpr static uint64_t kLogSectionHash = FNV(kLogSection);

  constexpr static std::string_view kScriptingSection = "SCRIPT-ENGINE";
  constexpr static uint64_t kScriptingSectionHash = FNV(kScriptingSection);

  constexpr static std::string_view kResourcesSection = "RESOURCES";
  constexpr static uint64_t kResourcesSectionHash = FNV(kResourcesSection);

  constexpr static std::string_view kScenesSection = "SCENES";
  constexpr static uint64_t kScenesSectionHash = FNV(kScenesSection);

  constexpr static std::string_view kMetadataSection = "METADATA";
  constexpr static uint64_t kMetadataSectionHash = FNV(kMetadataSection);

  /// useful value keys
  constexpr static std::string_view kNameValue = "NAME";
  constexpr static uint64_t kNameValueHash = FNV(kNameValue);

  constexpr static std::string_view kHeightValue = "HEIGHT";
  constexpr static uint64_t kHeightValueHash = FNV(kHeightValue);

  constexpr static std::string_view kWidthValue = "WIDTH";
  constexpr static uint64_t kWidthValueHash = FNV(kWidthValue);

  constexpr static std::string_view kTitleValue = "TITLE";
  constexpr static uint64_t kTitleValueHash = FNV(kTitleValue);

  constexpr static std::string_view kXposValue = "XPOS";
  constexpr static uint64_t kXposValueHash = FNV(kXposValue);

  constexpr static std::string_view kYposValue = "YPOS";
  constexpr static uint64_t kYposValueHash = FNV(kYposValue);
  
  constexpr static std::string_view kCenteredValue = "CENTERED";
  constexpr static uint64_t kCenteredValueHash = FNV(kCenteredValue);

  constexpr static std::string_view kClearColorValue = "CLEAR-COLOR";
  constexpr static uint64_t kClearColorValueHash = FNV(kClearColorValue);

  constexpr static std::string_view kClearBuffersValue = "CLEAR-BUFFERS";
  constexpr static uint64_t kClearBuffersValueHash = FNV(kClearBuffersValue);
  
  constexpr static std::string_view kColorValue = "COLOR";
  constexpr static uint64_t kColorValueHash = FNV(kColorValue);

  constexpr static std::string_view kDepthValue = "DEPTH";
  constexpr static uint64_t kDepthValueHash = FNV(kDepthValue);

  constexpr static std::string_view kStencilValue = "STENCIL";
  constexpr static uint64_t kStencilValueHash = FNV(kStencilValue);
  
  constexpr static std::string_view kResizableValue = "RESIZABLE";
  constexpr static uint64_t kResizableValueHash = FNV(kResizableValue);

  constexpr static std::string_view kBorderlessValue = "BORDERLESS";
  constexpr static uint64_t kBorderlessValueHash = FNV(kBorderlessValue);

  constexpr static std::string_view kMaximizedValue = "MAXIMIZED";
  constexpr static uint64_t kMaximizedValueHash = FNV(kMaximizedValue);

  constexpr static std::string_view kMinimizedValue = "MINIMIZED";
  constexpr static uint64_t kMinimizedValueHash = FNV(kMinimizedValue);

  constexpr static std::string_view kAllowHighDpiValue = "ALLOW-HIGH-DPI";
  constexpr static uint64_t kAllowHighDpiValueHash = FNV(kAllowHighDpiValue);

  constexpr static std::string_view kMajorValue = "MAJOR";
  constexpr static uint64_t kMajorValueHash = FNV(kMajorValue);
  
  constexpr static std::string_view kMinorValue = "MINOR";
  constexpr static uint64_t kMinorValueHash = FNV(kMinorValue);

  constexpr static std::string_view kDoubleBufferValue = "DOUBLE-BUFFER";
  constexpr static uint64_t kDoubleBufferValueHash = FNV(kDoubleBufferValue);

  constexpr static std::string_view kDepthSizeValue = "DEPTH-SIZE";
  constexpr static uint64_t kDepthSizeValueHash = FNV(kDepthSizeValue);

  constexpr static std::string_view kStencilSizeValue = "STENCIL-SIZE";
  constexpr static uint64_t kStencilSizeValueHash = FNV(kStencilSizeValue);

  constexpr static std::string_view kAccelVisualValue = "ACCELERATED-VISUAL";
  constexpr static uint64_t kAccelVisualValueHash = FNV(kAccelVisualValue);

  constexpr static std::string_view kMultisampleBuffersValue = "MULTISAMPLE-BUFFERS";
  constexpr static uint64_t kMultisampleBuffersValueHash = FNV(kMultisampleBuffersValue);

  constexpr static std::string_view kMultisampleSamplesValue = "MULTISAMPLE-SAMPLES";
  constexpr static uint64_t kMultisampleSamplesValueHash = FNV(kMultisampleSamplesValue);

  constexpr static std::string_view kSrgbCapableValue = "SRGB-CAPABLE";
  constexpr static uint64_t kSrgbCapableValueHash = FNV(kSrgbCapableValue);

  constexpr static std::string_view kVsyncValue = "VSYNC";
  constexpr static uint64_t kVsyncValueHash = FNV(kVsyncValue);

  constexpr static std::string_view kDepthTestValue = "DEPTH-TEST";
  constexpr static uint64_t kDepthTestValueHash = FNV(kDepthTestValue);

  constexpr static std::string_view kStencilTestValue = "STENCIL-TEST";
  constexpr static uint64_t kStencilTestValueHash = FNV(kStencilTestValue);
  
  constexpr static std::string_view kCullFaceValue = "CULL-FACE";
  constexpr static uint64_t kCullFaceValueHash = FNV(kCullFaceValue);

  constexpr static std::string_view kMultisampleValue = "MULTISAMPLE";
  constexpr static uint64_t kMultisampleValueHash = FNV(kMultisampleValue);

  constexpr static std::string_view kBlendValue = "BLEND";
  constexpr static uint64_t kBlendValueHash = FNV(kBlendValue);

  constexpr static std::string_view kDisabledValue = "DISABLED";
  constexpr static uint64_t kDisabledValueHash = FNV(kDisabledValue);

  constexpr static std::string_view kThemeValue = "THEME";
  constexpr static uint64_t kThemeValueHash = FNV(kThemeValue);

  constexpr static std::string_view kIniPathValue = "INI-PATH";
  constexpr static uint64_t kIniPathValueHash = FNV(kIniPathValue);

  constexpr static std::string_view kModulesValue = "MODULES";
  constexpr static uint64_t kModulesValueHash = FNV(kModulesValue);
  
  constexpr static std::string_view kLoadCoreModulesValue = "LOAD-CORE-MODULES";
  constexpr static uint64_t kLoadCoreModulesValueHash = FNV(kLoadCoreModulesValue);

  constexpr static std::string_view kEntitiesValue = "ENTITIES";
  constexpr static uint64_t kEntitiesValueHash = FNV(kEntitiesValue);

  constexpr static std::string_view kPathValue = "PATH";
  constexpr static uint64_t kPathValueHash = FNV(kPathValue);

  constexpr static std::string_view kUuidValue = "UUID";
  constexpr static uint64_t kUuidValueHash = FNV(kUuidValue);

  constexpr static std::string_view kComponentsValue = "COMPONENTS";
  constexpr static uint64_t kComponentsValueHash = FNV(kComponentsValue);
  
  constexpr static std::string_view kChildrenValue = "CHILDREN";
  constexpr static uint64_t kChildrenValueHash = FNV(kChildrenValue);

} // namespace other 

#endif // !OTHER_ENGINE_CONFIG_KEYS_HPP
