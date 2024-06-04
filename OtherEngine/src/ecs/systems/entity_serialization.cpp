/**
 * \file ecs/systems/entity_serialization.cpp
 **/
#include "ecs/systems/entity_serialization.hpp"

namespace other {

  void SerializerTable::InitializeTable(const Ref<Project>& project) {
    LoadBuiltinComponentSerializers();
  }
      
  void SerializerTable::LoadBuiltinComponentSerializers() {
  }

} // namespace other
