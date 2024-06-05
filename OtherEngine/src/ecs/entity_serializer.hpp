/**
 * \file ecs/entity_serializer.hpp
 **/
#ifndef OTHER_ENGINE_ENTITY_SERIALIZER_HPP
#define OTHER_ENGINE_ENTITY_SERIALIZER_HPP

#include "ecs/entity.hpp"

namespace other {

  class EntitySerializer {
    public:
      EntitySerializer() {}
      ~EntitySerializer() {}

      void Serialize(std::ostream& stream , Entity* entity) const;
      UUID Deserialize(Ref<Scene>& ctx , const std::string& name , const ConfigTable& scene_table) const;

    private:
  };

} // namespace other

#endif // !OTHER_ENGINE_ENTITY_SERIALIZER_HPP
