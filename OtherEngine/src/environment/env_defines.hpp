/**
 * \file environment/env_defines.hpp
 **/
#ifndef OTHER_ENGINE_ENV_DEFINES_HPP
#define OTHER_ENGINE_ENV_DEFINES_HPP

#include <cstdint>
#include <string_view>

#include <spdlog/fmt/fmt.h>

namespace other {

  struct address_t {
    uint8_t page;
    uint32_t page_idx;
  };
  static_assert(sizeof(address_t) == sizeof(uint64_t), "Address type must be 64 bits");

} // namespace other

template <>
struct fmt::formatter<other::address_t> : public fmt::formatter<std::string_view> {
  auto format(other::address_t address, fmt::format_context& ctx) {
    using namespace std::string_view_literals;
    std::string str = fmt::format("{}:{}"sv, address.page, address.page_idx);
    return fmt::formatter<std::string_view>::format(str,ctx);
  }
};

#endif // !OTHER_ENGINE_ENV_DEFINES_HPP
