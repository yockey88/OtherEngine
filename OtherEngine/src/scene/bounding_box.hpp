/**
 * \file scene/bounding_box.hpp
 **/
#ifndef OTHER_ENGINE_BOUNDING_BOX_HPP
#define OTHER_ENGINE_BOUNDING_BOX_HPP

#include <array>
#include <string_view>

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <spdlog/fmt/bundled/format.h>
#include <string_view>

#include "core/defines.hpp"
#include "math/vecmath.hpp"

namespace other {

  struct BoundingBox {
    glm::vec3 min = glm::vec3(0.f);
    glm::vec3 max = glm::vec3(0.f);
    glm::vec3 extent = glm::vec3(0.f);

    Opt<std::array<glm::vec3 , 8>> corners = std::nullopt;

    constexpr BoundingBox() {}
    constexpr BoundingBox(const glm::vec3& min , const glm::vec3& max) : min(min) , max(max) , extent(max - min) {}
    constexpr BoundingBox(const glm::vec3& point) : BoundingBox(point , point) {}

    bool Contains(const glm::vec3& point) const;
    bool OnBoundary(const glm::vec3& point) const;

    void ExpandToInclude(const glm::vec3& point);
    void ExpandToFill(const BoundingBox& other);

    glm::vec3 Center() const;
    bool Intersects(/* ray , near , far */) const { return false; }

    uint32_t MaxDimension() const;
    constexpr float SurfaceArea() const;

    const std::array<glm::vec3 , 8>& Corners();
  };

} // namespace other

template <>
struct fmt::formatter<other::BoundingBox> : fmt::formatter<std::string_view> {
  auto format(const other::BoundingBox& bbox , fmt::format_context& ctx) {
    std::stringstream ss;
    ss << other::fmtstr("BBOX(@min{} , @max{} , @extent{})" , bbox.min , bbox.max , bbox.extent);
    return fmt::formatter<std::string_view>::format(ss.str() , ctx);
  }
};

#endif // !OTHER_ENGINE_BOUNDING_BOX_HPP