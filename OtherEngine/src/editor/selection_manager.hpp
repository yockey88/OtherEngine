/**
 * \file editor/selection_manager.hpp
 **/
#ifndef OTHER_ENGINE_SELECTION_MANAGER_HPP
#define OTHER_ENGINE_SELECTION_MANAGER_HPP

namespace other {

  class Entity;

  class SelectionManager {
    public:
      static void Select(Entity* entity);

      static bool HasSelection();

      static void ClearSelection();

      static Entity* ActiveSelection();

    private:
      static Entity* entity_selection_context;
  };

} // namespace other

#endif // !OTHER_ENGINE_SELECTION_MANAGER_HPP
