/**
 * \file ecs/systems/component_database.hpp
 **/
#ifndef OTHER_ENGINE_COMPONENT_DATABASE_HPP
#define OTHER_ENGINE_COMPONENT_DATABASE_HPP

#include <utility>

#include <entt/core/type_info.hpp>
#include <reflection/type_database.hpp>

#include "ecs/component.hpp"
#include "ecs/components/tag.hpp"

namespace other {

  class ComponentDataBase {
   public:
    static std::string GetComponentTagUc(size_t idx);
    static std::string GetComponentTagLc(size_t idx);
    static int32_t GetComponentIdxFromTag(const std::string_view tag);

    static size_t GetComponentStride(size_t idx);

    template <typename T>
      requires ComponentType<T>
    void RegisterComponent() {
      T comp{};
      ComponentData& data = kComponentData[comp.component_idx];
      OE_ASSERT(data.metadata == nullptr, "Component already registered");

      data.metadata = comp.ReadMetadata();
    }

   private:
    struct ComponentData {
      const echo::TypeMetadata* metadata;
      const ComponentTag* tag;

      ComponentData()
          : metadata(nullptr), tag(nullptr) {}
      ComponentData(const ComponentTag* tag)
          : tag(tag) {}
    };

    static std::array<ComponentData, kNumComponents> kComponentData;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_COMPONENT_DATABASE_HPP