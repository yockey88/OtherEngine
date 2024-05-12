/**
 * \file editor/entity_properties.cpp
 **/
#include "editor/entity_properties.hpp"

#include <stdexcept>
#include <string_view>

#include <imgui/imgui.h>

#include "ecs/entity.hpp"
#include "editor/selection_manager.hpp"

namespace other {

  void EntityProperties::OnGuiRender(bool& is_open) {
    if (!is_open) {
      return;
    }

    if (!ImGui::Begin("Properties" , &is_open)) {
      ImGui::End();
      return;
    }

    Entity* selection = SelectionManager::ActiveSelection();
    if (selection == nullptr) {
      ///> If ActiveSelection is ever null then we are in major trouble, we deserve to crash
      throw std::runtime_error("ay yo wtf happened");
    }

    ImGui::Text("%s" , selection->Name().c_str());

    ImGui::End();
  }

  void EntityProperties::OnEvent(Event* e) {

  }

  void EntityProperties::OnProjectChange(const Ref<Project>& project) {

  }

  void EntityProperties::SetSceneContext(const Ref<Scene>& scene) {

  }

} // namespace other
