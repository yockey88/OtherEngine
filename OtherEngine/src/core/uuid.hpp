/**
 * \file core/uuid.hpp
*/
#ifndef OTHER_ENGINE_UUID_HPP
#define OTHER_ENGINE_UUID_HPP

#include <cstdint>
#include <functional>

#include <spdlog/fmt/fmt.h>

namespace other {

  struct UUID {
    public:
      constexpr UUID() : uuid(0) {}
      constexpr UUID(uint64_t uuid) : uuid(uuid) {}
      constexpr UUID(const UUID& other) : uuid(other.uuid) {}

      operator uint64_t() const { return uuid; }
      operator const uint64_t() const { return uuid; }

      uint64_t Get() const { return uuid; }

      constexpr auto operator<=>(const UUID&) const = default;
    
    private:
      uint64_t uuid;
  };

  struct UUID32 {
    public:
      constexpr UUID32() : uuid(0) {}
      constexpr UUID32(uint32_t uuid) : uuid(uuid) {}
      constexpr UUID32(const UUID32& other) : uuid(other.uuid) {}

      operator uint32_t() const { return uuid; }
      operator const uint32_t() const { return uuid; }

      uint32_t Get() const { return uuid; }

      constexpr auto operator<=>(const UUID32&) const = default;
    
    private:
      uint32_t uuid;
  };

} // namespace other

namespace std {

  template <>
  struct hash<other::UUID> {
    std::size_t operator()(const other::UUID& uuid) const {
      return std::hash<uint64_t>()(uuid.Get());
    }
  };

  template <>
  struct hash<other::UUID32> {
    std::size_t operator()(const other::UUID32& uuid) const {
      return std::hash<uint32_t>()(uuid.Get());
    }
  };

} // namespace std
  
template <>
struct fmt::formatter<other::UUID> : public fmt::formatter<std::string_view> {
  auto format(const other::UUID& uuid, fmt::format_context& ctx) {
    return fmt::formatter<std::string_view>::format(fmt::format(std::string_view{ "[{}:{:#08x}]" } , uuid.Get() , uuid.Get()) , ctx);
  }
};

template <>
struct fmt::formatter<other::UUID32> : public fmt::formatter<std::string_view> {
  auto format(const other::UUID32& uuid, fmt::format_context& ctx) {
    return fmt::formatter<std::string_view>::format(fmt::format(std::string_view{ "[{}:{:#04x}]" } , uuid.Get() , uuid.Get()) , ctx);
  }
};

#endif // !OTHER_ENGINE_UUID_HPP
