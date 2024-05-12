/**
 * \file editor/selection_manager.cpp
 **/
#include "editor/selection_manager.hpp"

#include "ecs/entity.hpp"

namespace other {
      
  Entity* SelectionManager::entity_selection_context = nullptr;

  void SelectionManager::Select(Entity* entity) {
    entity_selection_context = entity; 
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

} // namespace other
