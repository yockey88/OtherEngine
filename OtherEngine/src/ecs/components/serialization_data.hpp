/**
 * \file ecs/components/serialization_data.hpp
 **/
#ifndef OTHER_ENGINE_ENTITY_SERIALIZATION_DATA_HPP
#define OTHER_ENGINE_ENTITY_SERIALIZATION_DATA_HPP

#include <cstdint>
#include <set>

#include "ecs/component.hpp"

namespace other {

  struct SerializationData : public Component {
    std::set<int32_t> entity_components;
    ECS_COMPONENT(SerializationData , -1);
  }; 

} // namespace other

#endif // !OTHER_ENGINE_ENTITY_SERIALIZATION_DATA_HPP
