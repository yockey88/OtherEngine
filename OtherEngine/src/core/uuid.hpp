/**
 * \file core/uuid.hpp
*/
#ifndef OTHER_ENGINE_UUID_HPP
#define OTHER_ENGINE_UUID_HPP

#include <cstdint>
#include <functional>

#include <spdlog/fmt/fmt.h>

#include "core/defines.hpp"

namespace other {

  struct UUID {
    public:
      constexpr UUID() : uuid(0) {}
      constexpr UUID(uint64_t uuid) : uuid(uuid) {}
      constexpr UUID(const UUID& other) : uuid(other.uuid) {}

      operator uint64_t() const { return uuid; }
      operator const uint64_t() const { return uuid; }

      constexpr uint64_t Get() const { return uuid; }

      constexpr auto operator<=>(const UUID&) const = default;
    
    private:
      uint64_t uuid;
  };

  /// these return entity, because we should only ever be using this function for scripting
  ///   apis, their use elsewhere doesn't ever use UUIDs
  template <> 
  constexpr ValueType GetValueType<UUID>() {
    return ValueType::ENTITY;
  }

} // namespace other

namespace std {

  template <>
  struct hash<other::UUID> {
    std::size_t operator()(const other::UUID& uuid) const {
      return std::hash<uint64_t>()(uuid.Get());
    }
  };

} // namespace std
  
template <>
struct fmt::formatter<other::UUID> : public fmt::formatter<std::string_view> {
  auto format(const other::UUID& uuid, fmt::format_context& ctx) {
    return fmt::formatter<std::string_view>::format(fmt::format(std::string_view{ "[{}:{:#08x}]" } , uuid.Get() , uuid.Get()) , ctx);
  }
};

#endif // !OTHER_ENGINE_UUID_HPP
