/**
 * \file core/util.hpp
 **/
#include "core/logger.hpp"

namespace other {

  template <typename R, typename T>
  static constexpr R* CheckedCast(T* ptr) {
    OE_ASSERT(ptr != nullptr, "Invalid cast");
    try {
      return dynamic_cast<R*>(ptr);
    } catch (std::exception& e) {
      OE_CRITICAL("Failed to cast : {}", e.what());
    } catch (...) {
      OE_CRITICAL("Failed to cast : unknown error");
    }
    OE_ASSERT(false, "CheckedCast failed");
    return nullptr;
  }

}  // namespace other