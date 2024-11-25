/**
 * \file editor/selection_manager.cpp
 **/
#include "editor/selection_manager.hpp"

#include "ecs/entity.hpp"

namespace other {

  Entity* SelectionManager::entity_selection_context = nullptr;

  void SelectionManager::Select(Entity* entity) {
    OE_ASSERT(entity != nullptr, "Attempting to select null entity");
    if (entity_selection_context == entity) {
      return;
    }

    if (entity_selection_context != nullptr) {
      entity_selection_context->actively_selected = false;
    }
    entity_selection_context = entity;
    entity_selection_context->actively_selected = true;
  }

  bool SelectionManager::HasSelection() {
    return entity_selection_context != nullptr;
  }

  void SelectionManager::ClearSelection() {
    entity_selection_context = nullptr;
  }

  Entity* SelectionManager::ActiveSelection() {
    return entity_selection_context;
  }

}  // namespace other
