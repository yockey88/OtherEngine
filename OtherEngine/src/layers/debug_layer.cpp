/**
 * \file layers/debug_layer.cpp
 **/
#include "layers/debug_layer.hpp"

#include "rendering/ui/ui.hpp"
#include "scripting/script_engine.hpp"

namespace other {

  void DebugLayer::OnAttach() {
  }

  void DebugLayer::OnUpdate(float dt) {
    if (fps_data_index >= kFpsSamples) {
      fps_data_index = 0;
    }
    last_frame_time = dt;
    fps_data[fps_data_index++] = 1.0f / dt;
  }

  void DebugLayer::OnRender() {
  }

  void DebugLayer::OnUIRender() {
    auto showing = ImGui::Begin("DebugLayer");
    if (!showing) {
      ImGui::End();
      return;
    }

    ImGui::Text("FPS: %.1f", 1.0f / last_frame_time);
    ImGui::PlotHistogram("FPS", fps_data, kFpsSamples, fps_data_index, nullptr, 0.0f, 100.0f, ImVec2(0, 80));

    const auto& loaded_modules = ScriptEngine::GetModules();
    ImGui::Text("Loaded Modules: %llu", loaded_modules.size());
    for (const auto& module : loaded_modules) {
      ImGui::Text("  %s", module.second.name.c_str());
    }

    ImGui::End();
  }

  void DebugLayer::OnDetach() {
  }

} // namespace other
