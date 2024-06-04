/**
 * \file ecs/systems/entity_serialization.hpp
 **/
#ifndef OTHER_ENGINE_ENTITY_SERIALIZATION_HPP
#define OTHER_ENGINE_ENTITY_SERIALIZATION_HPP

#include <unordered_map>

#include "project/project.hpp"
#include "ecs/component_serializer.hpp"

namespace other {

  class SerializerTable {
    public:
      SerializerTable() = default;
      ~SerializerTable() {}

      void InitializeTable(const Ref<Project>& project);

    private:
      std::unordered_map<uint64_t , Scope<ComponentSerializer>> serializers;

      void LoadBuiltinComponentSerializers();
  };

} // namespace other

#endif // !OTHER_ENGINE_ENTITY_SERIALIZATION_HPP
